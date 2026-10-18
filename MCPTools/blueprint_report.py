exec(open("E:/Projects/unreal_projects/KBS/MCPTools/_common.py").read())

_path = args.get("path", DEFAULT_PATH)
_limit = args.get("limit", 20)

_blueprints = [d for d in find_assets(_path) if class_of(d) == "Blueprint"]
_lines = ["%d Blueprint(s) under %s (showing up to %d)" % (len(_blueprints), _path, _limit)]
for _d in _blueprints[:_limit]:
    _blueprint = load(_d)
    _generated = _blueprint.generated_class()
    _parent = _blueprint.get_blueprint_parent_class()
    _cdo = unreal.get_default_object(_generated)
    _parent_cdo = unreal.get_default_object(_parent)
    _lines.append("== %s  parent=%s" % (package_of(_d), _parent.get_name()))
    _excluded = set(dir(unreal.Actor if isinstance(_cdo, unreal.Actor) else unreal.Object))
    _theirs = editor_properties(_parent_cdo, _excluded)
    for _name, _value in editor_properties(_cdo, _excluded).items():
        _text, _parent_text = stable_text(_value), stable_text(_theirs.get(_name))
        if _name in _theirs and _text != _parent_text and "delegate" not in _text:
            _lines.append("  %s: %s (parent: %s)" % (_name, _text, _parent_text))
emit(_lines)
