exec(open("E:/Projects/unreal_projects/KBS/MCPTools/_common.py").read())

require_tools()
_counts = {"updated": 0, "skipped": 0, "refused": 0}
_lines = []
_path = normalize_package(args["asset_path"])
if guard(_path):
    _counts["refused"] += 1
    _lines.append(guard(_path))
elif not asset_exists(_path):
    _counts["skipped"] += 1
    _lines.append("SKIP %s: asset does not exist" % _path)
else:
    if apply_properties(_path, args["properties"], is_apply(args), _counts, _lines):
        _counts["updated"] += 1
finish(args, _lines, _counts)
