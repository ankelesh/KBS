### TASK: Refactor tactical grid

## Goal
Implement all suggestions to the grid

## Path
Grid class: $TACT/Grid/TacBattleGrid{.h/.cpp}
Unit class: $UNIT/Units/Unit{.h/.cpp}

## Unit rules
Always divided on two teams : attackers and defenders

## Suggestions
1. I want a way to add Units to the grid in editor. Propose solution
2. Units on Flank cells must be turned to normal cell nearby (each flank has only one neighbour cell)
3. Use decal references instead of 25 separate decals if possible (say if will break visuals)

## Implementation Order
1. Analyze grid and suggestions
2. Propose possible implementation
3. Implement 

## Current State
- [ ] Structure proposed
- [ ] Implementation done
- [ ] Copy task file in ./claude/logs/{task name}.md

## Proposition guidelines
- Describe classes, members, relationships (no implementation)
- Note assumptions made
- Note dependencies on implemented systems