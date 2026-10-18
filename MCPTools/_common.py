import re
import warnings
import unreal

warnings.simplefilter("ignore")

ROOT = "E:/Projects/unreal_projects/KBS/MCPTools"
registry = unreal.AssetRegistryHelpers.get_asset_registry()
DEFAULT_PATH = "/Game/_KBS"

TOOLS_MISSING_MESSAGE = "KBSEditorTools module not loaded - build and restart the editor"
LIB = getattr(unreal, "KBSAssetToolsLibrary", None)
if LIB is None:
    print(TOOLS_MISSING_MESSAGE)


def require_tools():
    """Property get/set/dump live in the C++ KBSAssetToolsLibrary; scripts needing them call this first."""
    if LIB is None:
        raise RuntimeError(TOOLS_MISSING_MESSAGE)


def class_of(asset_data):
    return str(asset_data.asset_class_path.asset_name)


def package_of(asset_data):
    return str(asset_data.package_name)


def parent_class_name(asset_data):
    if class_of(asset_data) != "Blueprint":
        return None
    tag = str(asset_data.get_tag_value("ParentClass") or "")
    return tag.rsplit(".", 1)[-1].strip("'\"")


def find_assets(path=DEFAULT_PATH, class_name=None):
    found = []
    for data in registry.get_assets_by_path(path, recursive=True):
        if package_of(data).startswith("/Game/Fab") and not path.startswith("/Game/Fab"):
            continue
        if class_name is None or class_of(data) == class_name or parent_class_name(data) == class_name:
            found.append(data)
    return sorted(found, key=package_of)


def load(asset_data):
    return unreal.EditorAssetLibrary.load_asset(package_of(asset_data))


def stable_text(value):
    """str() without memory addresses so dumps are diffable across sessions."""
    return re.sub(r" \(0x[0-9A-Fa-f]+\)", "", str(value))


def inspectable(asset):
    """Returns (object whose properties to read, attribute names to exclude as base-class noise)."""
    if isinstance(asset, unreal.Blueprint):
        cdo = unreal.get_default_object(asset.generated_class())
        return cdo, set(dir(unreal.Actor if isinstance(cdo, unreal.Actor) else unreal.Object))
    return asset, set(dir(unreal.DataAsset))


def editor_properties(obj, excluded):
    props = {}
    for name in sorted(set(dir(obj)) - excluded):
        if name.startswith("_"):
            continue
        try:
            props[name] = obj.get_editor_property(name)
        except Exception:
            pass
    return props


def dump_asset_text(asset_data):
    obj, excluded = inspectable(load(asset_data))
    lines = ["== %s (%s)" % (package_of(asset_data), class_of(asset_data))]
    for name, value in editor_properties(obj, excluded).items():
        lines.append("  %s = %s" % (name, stable_text(value)))
    return "\n".join(lines)


def dump_all(path, class_name):
    return "\n".join(dump_asset_text(d) for d in find_assets(path, class_name))


def emit(lines, limit_chars=9000):
    text = "\n".join(lines)
    if len(text) > limit_chars:
        text = text[:limit_chars] + "\n... TRUNCATED (%d chars total); narrow path/class_name or lower limit" % len(text)
    print(text)


# ---------------------------------------------------------------- write side
SAFE_ROOT = "/Game/_KBS/"


def is_apply(args):
    return args.get("apply") is True


ACTION_KEYS = ("updated", "created", "moved", "deleted")


def finish(args, lines, counts):
    """Prints the banner (decided by what actually happened), the lines and the SUMMARY line."""
    if not is_apply(args):
        banner = "DRY RUN (nothing is written; pass apply=True)"
    elif any(counts.get(k, 0) for k in ACTION_KEYS):
        banner = "APPLIED" + (" (with refused/skipped items)" if counts.get("refused") or counts.get("skipped") else "")
    else:
        banner = "NOTHING APPLIED (everything was refused or skipped)"
    emit([banner] + lines)
    summary(counts)


def guard(path):
    """Returns None if path is writable, else a refusal message. Delegates to C++; no override exists."""
    if LIB is not None:
        return None if LIB.is_writable_path(str(path)) else "REFUSED %s: writes are only allowed under %s" % (path, SAFE_ROOT)
    p = str(path).split(".")[0].replace("\\", "/")
    if ".." in p.split("/") or not (p.rstrip("/") + "/").startswith(SAFE_ROOT):
        return "REFUSED %s: writes are only allowed under %s" % (path, SAFE_ROOT)
    return None


def refuse_existing(path, args, what):
    """Pre-existing assets are only touched by move/delete/overwrite when args["allow_existing"] is True."""
    if args.get("allow_existing") is True:
        return None
    return "REFUSED %s: %s of a pre-existing asset requires allow_existing=True" % (path, what)


def normalize_package(path):
    return str(path).split(".")[0].rstrip("/")


def asset_exists(path):
    return unreal.EditorAssetLibrary.does_asset_exist(normalize_package(path))


def referencers_of(package):
    options = unreal.AssetRegistryDependencyOptions()
    options.include_soft_package_references = True
    options.include_hard_package_references = True
    options.include_searchable_names = False
    options.include_soft_management_references = False
    options.include_hard_management_references = False
    return sorted(str(p) for p in (registry.get_referencers(package, options) or []) if str(p) != package)


def resolve_class(class_name):
    cls = getattr(unreal, class_name, None)
    if isinstance(cls, type):
        return cls
    for prefix in ("/Script/KBS.", "/Script/Engine.", "/Script/CoreUObject."):
        cls = unreal.load_class(None, prefix + class_name)
        if cls:
            return cls
    raise ValueError("class %s not found" % class_name)


def summary(counts):
    print("SUMMARY: " + ", ".join("%s=%d" % (k, v) for k, v in counts.items()))


def save(package):
    return unreal.EditorAssetLibrary.save_asset(normalize_package(package), only_if_is_dirty=False)


def to_text(value, nested=False):
    """Converts a simple Python value to UE property text. Strings pass through raw at top level."""
    if isinstance(value, bool):
        return "True" if value else "False"
    if isinstance(value, (int, float)):
        return repr(value)
    if value is None:
        return "None"
    if isinstance(value, dict):
        return "(" + ",".join("%s=%s" % (k, to_text(v, True)) for k, v in value.items()) + ")"
    if isinstance(value, (list, tuple, set)):
        return "(" + ",".join(to_text(v, True) for v in value) + ")"
    text = str(value)
    if nested and (not text or text.startswith("/") or re.search(r"[,()=\"'\s]", text)):
        return '"' + text.replace("\\", "\\\\").replace('"', '\\"') + '"'
    return text


_EDIT_BRACKET = re.compile(r"^(?P<path>.+?)\[(?P<op>\+\d*|-\d+|\*)\]$")
_EDIT_BRACE = re.compile(r"^(?P<path>.+?)\{(?P<op>.*)\}$")


def _unquote(key):
    key = key.strip()
    return key[1:-1] if len(key) >= 2 and key[0] == key[-1] == '"' else key


def apply_edit(asset_path, prop, value, apply, save_asset=True):
    """
    Edits one property through the C++ library. Returns (ok, message).
    prop syntax (see README): plain path, `arr[+]` append, `arr[+N]` insert at N, `arr[-N]` remove index N,
    `arr[*]` clear, `set{+}` add, `set{-}` remove, `map{key}` set/add entry, `map{-key}` remove entry.
    """
    require_tools()
    text = to_text(value)
    bracket = _EDIT_BRACKET.match(prop)
    brace = _EDIT_BRACE.match(prop)
    if bracket:
        path, op = bracket.group("path"), bracket.group("op")
        if op == "+":
            result = LIB.array_append(asset_path, path, text, apply, save_asset)
        elif op.startswith("+"):
            result = LIB.array_insert(asset_path, path, int(op[1:]), text, apply, save_asset)
        elif op == "*":
            result = LIB.array_clear(asset_path, path, apply, save_asset)
        else:
            result = LIB.array_remove_at(asset_path, path, int(op[1:]), apply, save_asset)
    elif brace:
        path, op = brace.group("path"), brace.group("op")
        if op == "+":
            result = LIB.set_add(asset_path, path, text, apply, save_asset)
        elif op == "-":
            result = LIB.set_remove(asset_path, path, text, apply, save_asset)
        elif op.startswith("-"):
            result = LIB.map_remove(asset_path, path, _unquote(op[1:]), apply, save_asset)
        else:
            result = LIB.map_set(asset_path, path, _unquote(op), text, apply, save_asset)
    else:
        result = LIB.set_property_text(asset_path, prop, text, apply, save_asset)
    return result.success, str(result.message)


def apply_properties(asset_path, properties, apply, counts, lines):
    """Applies/plans a {property: value} dict on one asset (package path). Returns True if anything changed."""
    changed = False
    for name, value in properties.items():
        ok, message = apply_edit(asset_path, name, value, apply)
        lines.append("%s [%s]" % (message, name) if ok else "SKIP %s.%s: %s" % (asset_path, name, message))
        if ok:
            changed = True
        elif message.startswith("REFUSED"):
            counts["refused"] += 1
        else:
            counts["skipped"] += 1
    return changed


def bulk_edit(args, apply):
    require_tools()
    prop, new = args["property_name"], args["new_value"]
    limit = args.get("limit", 50)
    path = args.get("path", DEFAULT_PATH)
    counts = {"updated": 0, "skipped": 0, "refused": 0}
    lines = ["%s -> %r" % (prop, new)]
    refusal = guard(path) if apply else None
    if refusal:
        counts["refused"] += 1
        lines.append(refusal)
        finish(args, lines, counts)
        return
    assets = find_assets(path, args.get("class_name"))
    lines[0] += " on %d asset(s)" % len(assets)
    for i, data in enumerate(assets):
        pkg = package_of(data)
        before = len(lines)
        if apply and guard(pkg):
            counts["refused"] += 1
            lines.append(guard(pkg))
            continue
        ok, message = apply_edit(pkg, prop, new, apply)
        lines.append(message if ok else "SKIP %s: %s" % (pkg, message))
        if ok:
            counts["updated"] += 1
        else:
            counts["skipped"] += 1
        if i >= limit:
            del lines[before:]
    if len(assets) > limit:
        lines.append("... listing truncated at limit=%d (all matches were processed)" % limit)
    finish(args, lines, counts)
