exec(open("E:/Projects/unreal_projects/KBS/MCPTools/_common.py").read())

_class_name = args.get("class_name")
_path = args.get("path", DEFAULT_PATH)
_limit = args.get("limit", 10)
_format = "json" if args.get("json") is True else args.get("format", "text")

_assets = find_assets(_path, _class_name)
_lines = ["%d asset(s) match class=%s path=%s (showing up to %d, format=%s)" % (len(_assets), _class_name, _path, _limit, _format)]
if _format != "text":
    require_tools()
for _d in _assets[:_limit]:
    if _format == "json":
        _r = LIB.dump_asset_json(package_of(_d), args.get("include_inherited") is True)
    elif _format == "native":
        _r = LIB.dump_asset_text(package_of(_d), args.get("include_inherited") is True)
    else:
        _lines.append(dump_asset_text(_d))
        continue
    _lines.append(str(_r.value) if _r.success else "== %s: %s" % (package_of(_d), _r.message))
emit(_lines, args.get("max_chars", 9000))
