# Project KBS (UE 5.8 / C++, Rider)

## Context
Turn-based tactical strategy game. Solo dev: strong C++, still learning UE idioms.
Architecture: gameplay logic in C++, exposed to Blueprints via thin wrappers.

## Paths
Headers: ./Source/KBS/Classes/<sub>/
Impl:    ./Source/KBS/Private/<sub>/
Subdirs (same under both):
- GameMechanics/            core mechanics
- GameMechanics/Tactical/   tactical combat
- GameMechanics/Units/      units
- GameplayTypes/            shared types/enums/structs
- UI/                       UI (headers only under Classes/UI/)
Tests: ./Source/KBSTests/
Main map: ./Content/BaseTestGround.umap

## Never touch
Intermediate/, Binaries/, Saved/, DerivedDataCache/, *.generated.h
Never edit .uasset/.umap as files (use the editor asset tooling below)

## Tooling (Rider MCP)
- Prefer Rider MCP tools over grep/file scans: symbol search, find usages, file structure.
- After editing, check Rider inspections/problems for touched files.
- Builds: only via Rider MCP, Development Editor, incremental. Never call UBT/Build.bat from shell.
- Report errors only; summarize warnings as a count unless asked.
- If UE editor is running (Live Coding), don't build — ask me.

## Asset tooling (editor MCP)
- Assets are read/edited only via the running editor: `ue_execute_python` + scripts in ./MCPTools/ (use the `ue-asset-tools` skill). Editor-API edits are allowed when the task needs them.
- Writes: /Game/_KBS/ only, never /Game/Fab/. Dry-run first, then apply. Existing assets need `allow_existing`.
- The Rider MCP build tool can report "failed without diagnostics" on successful builds; the real result is in %LOCALAPPDATA%\UnrealBuildTool\Log.txt ("Result:").
- New module/UCLASS changes need an editor restart; no builds while the editor runs.

## Coding rules
- Read .h for signatures; open .cpp only when implementation matters.
- Header-only change → don't mention .cpp.
- Contracts over defensive code: domain objects are never null.
  - No nullptr checks on owned members/subsystems.
  - Required subsystems: one checkf at init.
  - Required params: checkf, never graceful fallback.
- Self-documenting code; comments only for non-obvious intent.
- Header changes to UCLASS/USTRUCT layout need an editor restart (no Live Coding) — flag it.

## Collaboration
- Design/architecture ambiguity → ask before acting.
- Tool/command failure → try alternatives; after 2 failed attempts on the same problem, stop and report.
- Show where to add code; no full-file rewrites.
- Minimal explanations unless asked.

## Response format
1. **Plan** — bullets
2. **Result** — modified files
3. **Notes** — only if deviated from plan