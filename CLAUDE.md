## Project KBS (UE5/C++)
## Context
Strategy turn-based game. Learning developer.

## Paths
$SRC = ./Source/KBS/
$TST = ./Source/KBSTests/
$AST = ./Content/
$H = Classes/
$CPP = Private/
$GM = GameMechanics/
$TACT = $GM/Tactical/
$UNITS = $GM/Units/
$TYPES = GameplayTypes/
$UI = $H/UI/

Note: Headers in $SRC$H{path}, Impl in $SRC$CPP{path}
Main scene: $AST/BaseTestGround.umap

## Guidelines
- Ask, don't assume
- Show WHERE to add code, not full rewrites
- Self-doc code; critical comments only
- Minimal explanations unless requested
- If .h only change, skip .cpp mentions
- Do not read cpp's if you don't need implementation - use signatures from .h, they are valid.
- Do not check owned members and subsystems on nullptr. If you need to be sure that subsystem exists, make checkf once, on init.
- Prefer checkf to nullptr graceful handling - we have contract that if function needs domain object, it should never receive nullptr or fail hard.

## Response protocol
1. **Plan:** What I'll do (bullets)
2. **Result:** Modified [files]
3. **Notes:** Only if deviated

## Constraints
Don't build or test anything

## Error handling
Try alternatives automatically. If 2+ repeats fail, stop and report
