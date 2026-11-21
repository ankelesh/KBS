---
name: ue-asset-tools
description: Read, audit, create, edit, move or delete UE assets, data assets and Blueprint defaults in KBS through the live editor (Rider MCP + MCPTools scripts). Use for any task touching .uasset content.
---

# UE asset tools

Details and per-script examples: `MCPTools/README.md`. Read it before first use.

## Preconditions
- `ue_status` must say `connected: true`. If not, ask the user to start the editor.
- Run scripts with `ue_execute_python`, rootFolder `E:\Projects\unreal_projects\KBS`:
  `args = {...}; exec(open(r"E:/Projects/unreal_projects/KBS/MCPTools/<script>.py").read())`
- `unreal.KBSAssetToolsLibrary` missing => module not built; user must build and restart the editor.

## Scripts
`dump_assets` (add `json: True`), `find_references`, `audit_assets`, `snapshot_diff`, `blueprint_report`, `create_asset`, `update_asset`, `bulk_edit_preview` / `bulk_edit_apply`, `move_asset`, `delete_asset`, `folder_ops`.

## Workflow
1. Read first: `dump_assets` / `find_references` before changing anything.
2. Run the write script without `apply`; show the `WOULD ...` plan.
3. Re-run with `apply: True` only when the plan is right (bulk or destructive: confirm with the user).
4. Read back to verify, then report the `SUMMARY:` line.

## Syntax
- Property path: `data.base_stats.accuracy_multiplier`, `effects[2].magnitude`, `Map["key"]`.
- Values: UE text `(A=1,B="x")`, or simple Python values.
- Containers: `effects[+]` append, `effects[+N]` insert, `effects[-N]` remove, `effects[*]` clear, `tags{+}` / `tags{-}` set add/remove, `stats{key}` / `stats{-key}` map set/remove.

## Rules
- Writes only under `/Game/_KBS/`; never `/Game/Fab/`. Dry-run is the default.
- `move_asset`, `delete_asset`, `create_asset` overwrite need `allow_existing: True` and rewrite referencers: use throwaway assets for experiments under `/Game/_KBS/_MCPToolsTest/` and delete the folder afterwards.
- Never edit `.uasset`/`.umap` as files.
- Container-valued dumps can be long: pass `limit`.
