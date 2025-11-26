### TASK: Refactor tactical grid

## Goal
Implement all special rules for the tactical layer grid

## Path
Grid class: $TACT/Grid/TacBattleGrid{.h/.cpp}
Unit class: $UNIT/Units/Unit{.h/.cpp}

## Unit rules
Always divided on two teams : attackers and defenders

## Grid rules
Attacker side: rows [0,1]
Defender side: rows [3,4]
Neutral (center) line: row 2
Restricted cells: columns [0,5] for center line
Flank cells: columns [0,5] for each row except 2
Sky cells: in AirLayer, no division on flank/normal cell

# Grid functionality
Ground pathfinding: Unit can move only on it's own layer, only one cell per turn. It can swap with friendly units, but not enemies. Attacker unit can not step on attacker flank cells and vice versa. 
Air pathfinding: Unit can move on any non-restricted cell on air layer without restrictions. 
Grid must calculate valid cells to step into.
Target finding: Grid must calculate for given unit valid targets. It must ask unit about his weapons range (ETargetReach). When receiving ETargetReach, it must return list of target actors.

## Implementation Order
1. Analyze grid and new rules
2. Propose possible implementation of these rules
3. Implement 
4. Change debug outlining to show flank with violet outlining and don't show restricted cells at all

## Current State
- [ ] Structure proposed
- [ ] Implementation done
- [ ] Debug outlining done
- [ ] Copy task file in ./claude/logs/{task name}.md

## Proposition guidelines
- Describe classes, members, relationships (no implementation)
- Note assumptions made
- Note dependencies on implemented systems