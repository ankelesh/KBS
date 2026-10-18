exec(open("E:/Projects/unreal_projects/KBS/MCPTools/_common.py").read())

_counts = {"deleted": 0, "skipped": 0, "refused": 0}
_lines = []
_paths = [normalize_package(p) for p in (args.get("paths") or [args["asset_path"]])]
for _p in _paths:
    if guard(_p):
        _counts["refused"] += 1
        _lines.append(guard(_p))
        continue
    if not asset_exists(_p):
        _counts["skipped"] += 1
        _lines.append("SKIP %s: asset does not exist" % _p)
        continue
    if refuse_existing(_p, args, "delete"):
        _counts["refused"] += 1
        _lines.append(refuse_existing(_p, args, "delete"))
        continue
    _refs = [r for r in referencers_of(_p) if r not in _paths]
    if _refs and args.get("force") is not True:
        _counts["refused"] += 1
        _lines.append("REFUSED %s: %d referencer(s) (pass force=True):" % (_p, len(_refs)))
        _lines += ["  " + r for r in _refs[:args.get("limit", 25)]]
        continue
    if is_apply(args):
        if unreal.EditorAssetLibrary.delete_asset(_p):
            _counts["deleted"] += 1
            _lines.append("DELETED %s" % _p)
        else:
            _counts["skipped"] += 1
            _lines.append("SKIP %s: delete failed" % _p)
    else:
        _counts["deleted"] += 1
        _lines.append("WOULD delete %s (%d referencer(s))" % (_p, len(_refs)))
finish(args, _lines, _counts)
