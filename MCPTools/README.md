# MCPTools

Unreal Editor Python scripts (read and, with `apply`, write), run through `mcp__rider__ue_execute_python` (rootFolder `E:\Projects\unreal_projects\KBS`).
Each reads a global `args` dict. Output is capped well under 10k chars; `limit` bounds the listing. Assets default to `/Game/_KBS` (never `/Game/Fab` unless passed as `path`). `_common.py` is shared helper code loaded by each script. Snapshots go to `_out/` (git-ignored).

## dump_assets.py
Lists assets of a class (Blueprint assets match by their own parent class name too, reading the CDO) and prints all editor properties.
Options: `json: True` (or `format: "json"`) prints pretty nested JSON from `DumpAssetJson` with real UE property names; `format: "native"` uses `DumpAssetText` (one `Name = ExportText` line per property); `include_inherited: True` also dumps Engine base-class properties; `max_chars` raises the output cap. Default `format` is the original Python dump (keeps old snapshots diffable). `json`/`native` need the KBSEditorTools module.
```python
args = {"class_name": "CombatDescriptorDataAsset", "path": "/Game/_KBS", "limit": 10}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/dump_assets.py").read())
```

## find_references.py
Referencers and dependencies of one asset (`/Game/` packages only).
```python
args = {"asset_path": "/Game/_KBS/Data/Units/Weapons/Descriptors/Base/DA_Phys_AnyEnemy", "limit": 50}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/find_references.py").read())
```

## audit_assets.py
Broken references, redirectors, assets without referencers (report only), naming-prefix violations, empty string/name properties on data assets.
```python
args = {"path": "/Game/_KBS/Data", "limit": 25}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/audit_assets.py").read())
```

## snapshot_diff.py
`snapshot` saves the dump to `_out/<name>.txt`; `diff` prints a unified diff of the current dump against it.
```python
args = {"mode": "snapshot", "name": "descriptors", "class_name": "CombatDescriptorDataAsset", "path": "/Game/_KBS"}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/snapshot_diff.py").read())
args = {"mode": "diff", "name": "descriptors", "class_name": "CombatDescriptorDataAsset", "path": "/Game/_KBS"}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/snapshot_diff.py").read())
```

## bulk_edit_preview.py
DRY RUN ONLY. Prints `current -> proposed` per matching asset; writes nothing.
```python
args = {"class_name": "CombatDescriptorDataAsset", "path": "/Game/_KBS", "property_name": "data", "new_value": "x", "limit": 50}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/bulk_edit_preview.py").read())
```

## blueprint_report.py
Blueprint assets, parent class, and properties whose CDO value differs from the parent class CDO.
```python
args = {"path": "/Game/_KBS", "limit": 20}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/blueprint_report.py").read())
```

# KBSEditorTools (C++ module)

Property reflection work lives in the Editor module `Source/KBSEditorTools` (`unreal.KBSAssetToolsLibrary`). The Python scripts are thin wrappers; if the module is not loaded they print `KBSEditorTools module not loaded - build and restart the editor` and the property scripts stop. Move/delete/folder scripts keep working with a Python fallback guard.

Every function returns a result with `.success`, `.message` (`WOULD ...` / `APPLIED ...` / `REFUSED ...` / `ERROR ...`) and `.value` (payload of read functions).
```python
r = unreal.KBSAssetToolsLibrary.set_property_text("/Game/_KBS/_MCPToolsTest/DA_New", "data.base_stats.accuracy_multiplier", "1.5", False, False)
print(r.success, r.message)   # (asset_path, property_path, value_text, apply, save_asset); apply=False is a dry run
```
`set_instanced_struct(asset, property_path, struct_path, fields_text, apply, save)` sets an `FInstancedStruct` property (text import of those is silently dropped by `set_property_text`), e.g. `component_entries[0].config`, `/Script/KBS.UnitChargeComponentConfig`, `(Entries=((Tag=(TagName="X"),CounterMax=1)))`.
Functions: `is_writable_path`, `get_property_text`, `set_property_text`, `set_instanced_struct`, `get_container_num`, `array_append/insert/remove_at/clear`, `set_add/set_remove`, `map_set/map_remove`, `dump_asset_json`, `dump_asset_text`, `validate_asset`. All mutating ones take `(..., apply, save_asset)`, change nothing unless `apply` is true, save only the touched package, and refuse anything outside `/Game/_KBS/` (also in dry run).

## Property path syntax
`data.base_stats.accuracy_multiplier` (struct members), `effects[2].magnitude` (array element), `Map["key"]` (map value; key as UE text, quotes optional). Names are matched case-insensitively with underscores ignored, so snake_case (`guaranteed_hit`), `bGuaranteedHit` and `GuaranteedHit` are equivalent. Descending through a hard/instanced object reference works (`weapon.damage`).

## Values
Values are UE text: `(A=1,B="x")` for structs, `(1,2,3)` for whole containers, `Melee` or `EMyEnum::Melee` for enums, `/Game/_KBS/Foo.Foo` for object refs. The Python helpers also accept simple Python values: bool -> `True`/`False`, numbers, strings (raw), dict -> `(K=V,...)`, list -> `(a,b)`.

## Container edit syntax (`properties` keys / `property_name`)
| property | action | value |
|---|---|---|
| `effects` or `effects[2]` | replace whole value / element 2 | UE text |
| `effects[+]` | append | element |
| `effects[+N]` | insert at index N | element |
| `effects[-N]` | remove index N | ignored |
| `effects[*]` | clear | ignored |
| `tags{+}` / `tags{-}` | set add / remove | element |
| `stats{key}` | map set or add entry (`stats{"key"}`) | value |
| `stats{-key}` | map remove entry | ignored |

# Write side (Create / Update / Move / Delete)

All write scripts take `apply=True` to act; without it they print `WOULD ...` under a `DRY RUN` banner and change nothing. Each ends with a `SUMMARY:` counts line. Only the assets actually changed are saved. The banner is `APPLIED` only if something was really changed; if everything was refused or skipped it says `NOTHING APPLIED`.

## Safety
- Dry-run by default: only `args["apply"] is True` writes. Output says `DRY RUN` or `APPLIED`.
- Scope guard (`guard()` in `_common.py`, delegating to `KBSAssetToolsLibrary.is_writable_path`): every written path must start with `/Game/_KBS/`; `/Game/Fab/`, `/Engine/`, `/Script/`, anything else (and `..`) is refused, no override. The C++ property functions apply it themselves, also for dry runs.
- Pre-existing assets: plain property edits on an existing asset are the normal use case (`apply=True` only). `move_asset`, `delete_asset` and `create_asset` with `overwrite` refuse any pre-existing asset unless `allow_existing=True` is passed (dry run included, so the preview shows the refusal). A move rewrites and re-saves the moved asset and all its referencers; experiment only on throwaway assets.
- Create never overwrites and delete never removes referenced assets unless `overwrite` / `force` is True.

## create_asset.py
`path`, `class_name` (DataAsset subclass uses DataAssetFactory) or `copy_from`, optional `properties`, `overwrite` (also needs `allow_existing=True`).
```python
args = {"path": "/Game/_KBS/_MCPToolsTest/DA_New", "copy_from": "/Game/_KBS/Data/.../DA_X", "apply": True}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/create_asset.py").read())
```

## update_asset.py
`asset_path`, `properties` {name: value}. Names are property paths (see above) and may use the container edit syntax (`effects[+]`, `tags{-}`, `stats{key}`); values are UE text or simple Python values.
```python
args = {"asset_path": "/Game/_KBS/_MCPToolsTest/DA_New", "properties": {"data.guaranteed_hit": True, "effects[+]": "/Game/_KBS/Data/Effects/DA_Burn.DA_Burn"}, "apply": True}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/update_asset.py").read())
```

## bulk_edit_apply.py
Same args as `bulk_edit_preview.py` (`property_name` may be a dotted path or use the container edit syntax) plus `apply`. Matching/conversion is shared in `_common.bulk_edit`.
```python
args = {"class_name": "CombatDescriptorDataAsset", "path": "/Game/_KBS/_MCPToolsTest", "property_name": "data.guaranteed_hit", "new_value": True, "apply": True}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/bulk_edit_apply.py").read())
```

## move_asset.py
`src`, `dst` (asset or folder), optional `fixup_redirectors`. Requires `allow_existing=True` (the source always pre-exists). Uses rename_asset / rename_directory; lists leftover redirectors and fixes them up on request.
```python
args = {"src": "/Game/_KBS/_MCPToolsTest/A", "dst": "/Game/_KBS/_MCPToolsTest/Sub/A", "fixup_redirectors": True, "apply": True}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/move_asset.py").read())
```

## delete_asset.py
`asset_path` or `paths`, optional `force`. Requires `allow_existing=True`. Refuses (and lists referencers) if anything outside the deleted set references the asset.
```python
args = {"asset_path": "/Game/_KBS/_MCPToolsTest/A", "apply": True}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/delete_asset.py").read())
```

## folder_ops.py
`op` = `create` | `delete` | `list`, `path`, `force` (delete of a non-empty folder).
```python
args = {"op": "create", "path": "/Game/_KBS/_MCPToolsTest", "apply": True}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/folder_ops.py").read())
```
