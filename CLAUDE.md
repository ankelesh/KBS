## Project KBS (UE5/C++)
## Context
Strategy turn-based game. Learning developer.

## Paths
$CMD = E:\Projects\KBS_tasks\Tasks
$SRC = ./Source/KBS/
$TST = ./Source/KBSTests/
$AST = ./Content/
$H = Classes/
$CPP = Private/
$GM = GameMechanics/
$TACT = $GM/Tactical/
$STRAT = $GM/Strategic/
$UNITS = $GM/Units/
$UABILITY = $UNITS/Abilities
$CMP = $TACT/Grid/Components/
$TYPES = GameplayTypes/

Note: Headers in $SRC$H{path}, Impl in $SRC$CPP{path}
Main scene: $AST/BaseTestGround.umap
Main orchestrator (tactical): $TACT/Grid/TacBattleGrid

## Guidelines
- Ask, don't assume
- Show WHERE to add code, not full rewrites
- Self-doc code; critical comments only
- Minimal explanations unless requested
- If .h only change, skip .cpp mentions

## Response protocol
1. **Plan:** What I'll do (bullets)
2. **Result:** Modified [files]
3. **Notes:** Only if deviated

## Constraints
Ask before modifying .uproject, project settings, or build files
Don't build anything

## Doc links
Only when requested or proposing solution

## Error handling
Try alternatives automatically. If 2+ repeats fail, stop and report

## After task complete
Copy task file to $CMD/task_logs/