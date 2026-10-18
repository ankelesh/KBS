exec(open("E:/Projects/unreal_projects/KBS/MCPTools/_common.py").read())

_counts = {"created": 0, "deleted": 0, "skipped": 0, "refused": 0}
_op = args["op"]
_path = normalize_package(args["path"])
_lines = []
if _op == "list":
    _subs = unreal.EditorAssetLibrary.list_assets(_path, recursive=False, include_folder=True)
    _lines += ["  " + str(s) for s in sorted(_subs)[:args.get("limit", 100)]]
elif guard(_path):
    _counts["refused"] += 1
    _lines.append(guard(_path))
elif _op == "create":
    if unreal.EditorAssetLibrary.does_directory_exist(_path):
        _counts["skipped"] += 1
        _lines.append("SKIP %s: already exists" % _path)
    elif is_apply(args):
        _counts["created"] += int(unreal.EditorAssetLibrary.make_directory(_path))
        _lines.append("CREATED %s" % _path)
    else:
        _counts["created"] += 1
        _lines.append("WOULD create %s" % _path)
elif _op == "delete":
    if not unreal.EditorAssetLibrary.does_directory_exist(_path):
        _counts["skipped"] += 1
        _lines.append("SKIP %s: does not exist" % _path)
    else:
        _content = list(unreal.EditorAssetLibrary.list_assets(_path, recursive=True, include_folder=False))
        if _content and args.get("force") is not True:
            _counts["refused"] += 1
            _lines.append("REFUSED %s: contains %d asset(s) (pass force=True)" % (_path, len(_content)))
            _lines += ["  " + str(a) for a in _content[:25]]
        elif is_apply(args):
            _counts["deleted"] += int(unreal.EditorAssetLibrary.delete_directory(_path))
            _lines.append("DELETED %s (%d asset(s))" % (_path, len(_content)))
        else:
            _counts["deleted"] += 1
            _lines.append("WOULD delete %s (%d asset(s))" % (_path, len(_content)))
else:
    _lines.append("unknown op %r" % _op)
if _op == "list":
    emit(["LIST"] + _lines)
    summary(_counts)
else:
    finish(args, _lines, _counts)
