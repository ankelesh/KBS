### TASK: Battlefield System

## Goal
Create 2-layer 5x5 battlefield grid. Exclude cells (3,1) and (3,5). Hold Unit actors.

## Technical
- Grid: 5x5, 2 layers (ground, air)
- Excluded: Row 3, Cols 1 and 5
- Unit: AActor-derived class
- Storage: propose
- Coordinate system:  0-indexed, column-major
- No visual implementation
- Class name: TacBattleGrid
- API scope: placement, retrieval, validation only

## Implementation Order
1. Propose class structure
2. Create class + grid

## Current State
- [ ] Implementation proposed
- [ ] Battlefield class created
- [ ] Copy task file in ./claude/log/{task name}.md

## Proposition guidelines
- Describe classes, members, relationships (no implementation)
- Explain storage choice rationale