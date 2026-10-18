exec(open("E:/Projects/unreal_projects/KBS/MCPTools/_common.py").read())

_counts = {"moved": 0, "skipped": 0, "refused": 0}
_lines = []
_src = normalize_package(args["src"])
_dst = normalize_package(args["dst"])
_is_dir = unreal.EditorAssetLibrary.does_directory_exist(_src) and not asset_exists(_src)
if guard(_src) or guard(_dst):
    _counts["refused"] += 1
    _lines += [m for m in (guard(_src), guard(_dst)) if m]
elif refuse_existing(_src, args, "move") and (_is_dir or asset_exists(_src)):
    _counts["refused"] += 1
    _lines.append(refuse_existing(_src, args, "move"))
elif not _is_dir and not asset_exists(_src):
    _counts["skipped"] += 1
    _lines.append("SKIP %s: does not exist" % _src)
elif (unreal.EditorAssetLibrary.does_directory_exist(_dst) if _is_dir else asset_exists(_dst)):
    _counts["refused"] += 1
    _lines.append("REFUSED: destination %s already exists" % _dst)
else:
    _n = len(registry.get_assets_by_path(_src, recursive=True)) if _is_dir else 1
    if not is_apply(args):
        _lines.append("WOULD move %s %s -> %s (%d asset(s))" % ("folder" if _is_dir else "asset", _src, _dst, _n))
        _counts["moved"] += _n
    else:
        _ok = unreal.EditorAssetLibrary.rename_directory(_src, _dst) if _is_dir else unreal.EditorAssetLibrary.rename_asset(_src, _dst)
        if not _ok:
            _counts["skipped"] += 1
            _lines.append("SKIP: rename failed %s -> %s" % (_src, _dst))
        else:
            _counts["moved"] += _n
            _lines.append("MOVED %s -> %s (%d asset(s))" % (_src, _dst, _n))
            _scan = _src.rsplit("/", 1)[0]
            _redirs = [d for d in registry.get_assets_by_path(_scan, recursive=True)
                       if class_of(d) == "ObjectRedirector" and package_of(d).startswith(_src)]
            _lines.append("Redirectors left at source: %d" % len(_redirs))
            _lines += ["  " + package_of(d) for d in _redirs[:25]]
            if args.get("fixup_redirectors") is True and _redirs:
                unreal.AssetToolsHelpers.get_asset_tools().fixup_referencers([d.get_asset() for d in _redirs], False)
                _lines.append("Fixed up %d redirector(s)" % len(_redirs))
finish(args, _lines, _counts)
