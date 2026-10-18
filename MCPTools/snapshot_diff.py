import difflib
import os
exec(open("E:/Projects/unreal_projects/KBS/MCPTools/_common.py").read())

_file = "%s/_out/%s.txt" % (ROOT, args["name"])
_current = dump_all(args.get("path", DEFAULT_PATH), args.get("class_name"))

if args["mode"] == "snapshot":
    os.makedirs(os.path.dirname(_file), exist_ok=True)
    with open(_file, "w", encoding="utf-8") as _f:
        _f.write(_current)
    print("Snapshot written: %s (%d lines)" % (_file, _current.count("\n") + 1))
else:
    with open(_file, encoding="utf-8") as _f:
        _saved = _f.read()
    _diff = list(difflib.unified_diff(_saved.splitlines(), _current.splitlines(), "snapshot", "current", lineterm=""))
    emit(_diff or ["No differences from snapshot " + args["name"]])
