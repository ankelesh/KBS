### TASK: Unit System

## Goal
Create Weapon class (Unit component) and BattleEffect (Weapon component)

## Path
Units/Unit.h
Units/Weapons/Weapon.h (create)
Units/Weapons/BattleEffect/BattleEffect.h (create)
Units/DamageTypes.h

## Technical
- Encapsulates weapon parameters
- Informative, actual battle calculations happen in DamageFormula (not implemented yet)
- Unit can have more than one weapon
- Weapon sometimes has BattleEffects applied on-hit 
- BattleEffect can be applied on targets or on weapon wielder
- Enum behavior: global -> separate file, local -> Weapon file

## Weapon stats
- Damage source (can be composite, like Fire+Earth, propose storage)
- If composite, DamageFormula will pick most efficient source
- Modified damage source
- Damage amount
- Modified damage amount (by buffs)
- Accuracy multiplier
- Recalculation is called by owner Unit

# Damage reach
- Closest enemies, Any enemy, All enemies, Area (cells), Flank, Any Friendly, All friendlies
- Closest enemies = not only in adjascent cells, but in same Layer, propose calc
- Any enemy = any enemy in any layer
- All enemies = any enemy alltogether
- Area = any unit in area, shape of area is designated using relative cell indices, propose structure
- Flank = adjascent cells on left and right side of the unit and cells closest to grid center, orientation always by grid top
- Any Friendly = any friendly in any layer
- All friendlies = Any Friendly alltogether

# BattleEffects
- A list of BattleEffect

## BattleEffect
- Virtual base for future effect types
- Has: Damage source


## Implementation Order
1. Analyze stats for edge cases and data structure needs
2. Propose implementation

## Current State
- [ ] Analysis complete
- [ ] Implementation proposed
- [ ] Copy task file in ./claude/logs/{task name}.md

## Proposition guidelines
- Describe classes, members, relationships (no implementation)
- Explain storage choice rationale
- Note assumptions made
- Note dependencies on implemented systems