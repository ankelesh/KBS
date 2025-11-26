### TASK: Unit System

## Goal
Alter Unit class to add characteristic system (base + modified sets)

## Path
/GameMechanics/Units/Unit.h

## Technical
- Two characteristic sets: base (recalc source), modified (runtime)
- Modified recalc: when buff/debuff action happens
- Game has 8 damage sources: Physical, Fire, Earth, Air, Water, Life, Death, Mind
- Life damage: no block/ward/reduction/immunity possible
- Weapons define damage (later)
- Enum behavior: if global (like damage sources) then to separate file, else to Unit file

## Unit characteristics

# Base
- Max health (int, usual range from 20 to 600)
- Current health
- Initiative (int, range from 0 to 100)
- Accuracy (percent, range from 0 to 100)

# Experience
- Total experience (int)
- Experience to level-up (int)
- Unit tier (int) - static value for a designated unit type
- Unit level on current tier (int) 

# Unit progression
- if there are another tiers set in separate file, unit will be replaced
- replacement means actor change by outer TierManager
- if there are no tiers, unit will gain a level

# Defense (propose structures)
- Wards (block 1 attack/source, consumable)
- Immunity (block all/source)
- Armour (%, reduces damage/source, floor at 0)
- Damage reduction (flat int, all sources)

# Level up behavior
- Base health *10% and current health * 10%
- Accuracy += 1%



## Implementation Order
1. Analyze characteristics for edge cases and data structure needs
2. Propose implementation

## Current State
- [ ] Analysis complete
- [ ] Implementation proposed
- [ ] Copy task file in ./claude/logs/{task name}.md

## Proposition guidelines
- Describe classes, members, relationships (no implementation)
- Explain storage choice rationale
- Note assumptions made