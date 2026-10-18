exec(open("E:/Projects/unreal_projects/KBS/MCPTools/_common.py").read())

_limit = args.get("limit", 50)
_package = args["asset_path"].split(".")[0]

_options = unreal.AssetRegistryDependencyOptions()
_options.include_soft_package_references = True
_options.include_hard_package_references = True
_options.include_searchable_names = False
_options.include_soft_management_references = False
_options.include_hard_management_references = False


def _project_only(packages):
    return sorted(str(p) for p in packages if str(p).startswith("/Game/"))


_refs = _project_only(registry.get_referencers(_package, _options) or [])
_deps = _project_only(registry.get_dependencies(_package, _options) or [])
_lines = ["Referencers (%d):" % len(_refs)] + ["  " + p for p in _refs[:_limit]]
_lines += ["Dependencies (%d):" % len(_deps)] + ["  " + p for p in _deps[:_limit]]
if max(len(_refs), len(_deps)) > _limit:
    _lines.append("... truncated at limit=%d" % _limit)
emit(_lines)
