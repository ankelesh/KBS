exec(open("E:/Projects/unreal_projects/KBS/MCPTools/_common.py").read())

_counts = {"created": 0, "skipped": 0, "refused": 0}
_lines = []
if args.get("properties"):
    require_tools()
_path = normalize_package(args["path"])
_name = _path.rsplit("/", 1)[-1]
_folder = _path.rsplit("/", 1)[0]
_copy = args.get("copy_from")
_exists = asset_exists(_path)
if guard(_path):
    _counts["refused"] += 1
    _lines.append(guard(_path))
elif _exists and args.get("overwrite") is not True:
    _counts["refused"] += 1
    _lines.append("REFUSED %s: already exists (pass overwrite=True and allow_existing=True)" % _path)
elif _exists and refuse_existing(_path, args, "overwrite"):
    _counts["refused"] += 1
    _lines.append(refuse_existing(_path, args, "overwrite"))
elif not _copy and "class_name" not in args:
    _counts["refused"] += 1
    _lines.append("REFUSED: need class_name or copy_from")
elif _copy and not asset_exists(_copy):
    _counts["skipped"] += 1
    _lines.append("SKIP: copy_from %s does not exist" % _copy)
else:
    _source = ("copy of " + _copy) if _copy else args["class_name"]
    if not is_apply(args):
        _lines.append("WOULD create %s from %s" % (_path, _source))
        for _k, _v in (args.get("properties") or {}).items():
            _lines.append("WOULD set %s.%s = %r" % (_path, _k, _v))
        _counts["created"] += 1
    else:
        if _exists:
            unreal.EditorAssetLibrary.delete_asset(_path)
        if _copy:
            _asset = unreal.EditorAssetLibrary.duplicate_asset(normalize_package(_copy), _path)
        else:
            _cls = resolve_class(args["class_name"])
            _factory = unreal.DataAssetFactory() if issubclass(_cls, unreal.DataAsset) else None
            if _factory:
                _factory.set_editor_property("data_asset_class", _cls)
            _asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(_name, _folder, _cls, _factory)
        if _asset is None:
            _counts["skipped"] += 1
            _lines.append("SKIP: creation failed for %s" % _path)
        else:
            _counts["created"] += 1
            _lines.append("CREATED %s from %s" % (_path, _source))
            save(_path)
            if args.get("properties"):
                apply_properties(_path, args["properties"], True, _counts, _lines)
finish(args, _lines, _counts)
