exec(open("E:/Projects/unreal_projects/KBS/MCPTools/_common.py").read())

_path = args.get("path", DEFAULT_PATH)
_limit = args.get("limit", 25)

_PREFIXES = {
    "Blueprint": ("BP_",),
    "WidgetBlueprint": ("WBP_",),
    "AnimBlueprint": ("ABP_",),
    "Material": ("M_",),
    "MaterialInstanceConstant": ("MI_",),
    "MaterialFunction": ("MF_",),
    "Texture2D": ("T_",),
    "StaticMesh": ("SM_",),
    "SkeletalMesh": ("SK_", "SKM_"),
    "AnimSequence": ("A_", "AS_", "Anim_"),
    "AnimMontage": ("AM_",),
    "NiagaraSystem": ("NS_",),
    "SoundWave": ("S_", "SW_"),
    "SoundCue": ("SC_",),
}
_options = unreal.AssetRegistryDependencyOptions()
_options.include_soft_package_references = False
_options.include_hard_package_references = True
_options.include_searchable_names = False
_options.include_soft_management_references = False
_options.include_hard_management_references = False

_assets = find_assets(_path)
_broken, _redirectors, _unreferenced, _naming, _empty = [], [], [], [], []

for _d in _assets:
    _pkg, _cls, _name = package_of(_d), class_of(_d), str(_d.asset_name)
    if _cls == "ObjectRedirector":
        _redirectors.append(_pkg)
        continue
    for _dep in registry.get_dependencies(_pkg, _options) or []:
        _dep = str(_dep)
        if _dep.startswith("/Game/") and not registry.get_assets_by_package_name(_dep):
            _broken.append("%s -> %s" % (_pkg, _dep))
    if not registry.get_referencers(_pkg, _options):
        _unreferenced.append(_pkg)
    _expected = _PREFIXES.get(_cls)
    if _expected and not _name.startswith(_expected):
        _naming.append("%s (%s) expected %s" % (_pkg, _cls, "/".join(_expected)))

    if _cls == "Blueprint":
        continue
    _obj = load(_d)
    if not isinstance(_obj, unreal.DataAsset):
        continue
    if not _name.startswith("DA_"):
        _naming.append("%s (%s) expected DA_" % (_pkg, _cls))
    for _prop, _val in editor_properties(_obj, set(dir(unreal.DataAsset))).items():
        if isinstance(_val, (str, unreal.Name, unreal.Text)) and str(_val) in ("", "None"):
            _empty.append("%s.%s" % (_pkg, _prop))


def _section(title, items):
    shown = ["  " + i for i in items[:_limit]]
    if len(items) > _limit:
        shown.append("  ... truncated at limit=%d" % _limit)
    return ["%s (%d):" % (title, len(items))] + shown


_lines = ["Audit of %s: %d assets" % (_path, len(_assets))]
_lines += _section("Broken references", _broken)
_lines += _section("Redirectors", _redirectors)
_lines += _section("No referencers (unused candidates, report only)", _unreferenced)
_lines += _section("Naming violations", sorted(set(_naming)))
_lines += _section("Empty string/name props on data assets", _empty)
emit(_lines)
