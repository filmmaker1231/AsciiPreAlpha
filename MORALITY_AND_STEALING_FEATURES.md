# Morality and Stealing Features

## Overview
This document describes the newly implemented morality system and stealing mechanics in the game.

## Morality System

### Basic Mechanics
- **Starting Value**: Units start with morality at 100
- **Range**: Morality ranges from 0 (minimum) to 100 (maximum)
- **Update Frequency**: Morality is updated every 1 second (1000ms)

### Morality Changes
1. **Hunger Below 50**: Morality decreases by 1 per second
2. **Hunger Above 50**: Morality increases by 1 per second
3. **Hunger Exactly 50**: Morality remains unchanged

### Implementation Details
- New field added to Unit class: `int morality = 100`
- New timestamp field: `Uint32 lastMoralityUpdate = 0`
- Update logic in GameLoop.cpp checks hunger level and adjusts morality accordingly

## Stealing Mechanics

### When Units Steal
A unit will attempt to steal food when **ALL** of the following conditions are met:
1. Unit's morality is below 10 (`morality < 10`)
2. Unit's hunger is 30 or below (`hunger <= 30`)
3. There is at least one house with food available in the world

### Stealing Behavior
1. **Priority**: Stealing has priority 8 in the action queue (same as EatFromHouse)
2. **Target Selection**: Unit finds the nearest house with food (any house, not just their own)
3. **Navigation**: Unit paths to the target house
4. **Consumption**: Unit eats one food item from the house storage
5. **Seed Drops**: When food is stolen and eaten:
   - 1-2 seeds drop at the eating location (85% chance for 1 seed, 15% chance for 2 seeds)
   - **Important**: Seeds remain owned by the house owner (the victim), not the thief
6. **Hunger Restoration**: Unit's hunger is restored to 100 after eating

### Messages
When stealing occurs, the following message is printed to console:
```
Food stolen from home (X, Y) by UnitName
```

Where:
- `(X, Y)` is the grid position of the house being stolen from
- `UnitName` is the name of the stealing unit

Additional debug messages are printed for seed drops:
```
Dropped seed {SEED_ID} in house at (X, Y) owned by unit {OWNER_ID}
```
Note: The values in braces `{}` are placeholders that will be replaced with actual values at runtime.

## Action Priority System

Updated priority list:
- Priority 1: Wander (lowest)
- Priority 3: CollectSeed
- Priority 4: BuildFarm, PlantSeed, HarvestFood
- Priority 5: BringItemToHouse (food)
- Priority 8: BuildHouse, EatFromHouse, **StealFood** (new)
- Priority 9: Eat (from world)

Higher priority actions interrupt lower priority actions.

## Testing Scenarios

### Scenario 1: Morality Decrease
1. Spawn a unit with U+Click
2. Wait for hunger to drop below 50
3. Observe morality decreasing by 1 per second (visible in logs if you add debug prints)
4. Morality will eventually reach 0 if hunger stays below 50

### Scenario 2: Morality Recovery
1. Feed a unit (bringing their hunger above 50)
2. Observe morality increasing by 1 per second
3. Morality will eventually reach 100 if hunger stays above 50

### Scenario 3: Stealing from Another Unit's House
1. Spawn Unit A with U+Click
2. Spawn food near Unit A with F+Click
3. Wait for Unit A to build a house and store food
4. Spawn Unit B far from food sources
5. Wait for Unit B's hunger to drop to 30 or below
6. Wait for Unit B's morality to drop below 10 (takes time!)
7. **Expected Result**: Unit B will path to Unit A's house and steal food
8. **Expected Output**: "Food stolen from home (X, Y) by UnitB"
9. Seeds dropped will be owned by Unit A (the house owner)

### Scenario 4: Preventing Stealing
1. If a unit has their own house with food, they will eat from it before stealing
2. Priority 8 (EatFromHouse and StealFood) means they're equally prioritized
3. But the EatFromHouse check happens first in the code, preventing unnecessary stealing

## Technical Implementation

### Files Modified
1. **Unit.h**: Added `morality` and `lastMoralityUpdate` fields
2. **Actions.h**: Added `StealFood` action type
3. **GameLoop.cpp**: 
   - Added morality update logic
   - Added stealing trigger logic
   - Modified food seeking to skip when stealing
4. **Unit.cpp**: 
   - Added `ActionType::StealFood` case in `processAction()`
   - Implements nearest house finding, navigation, and stealing

### Code Quality
- Follows existing code patterns and conventions
- Uses Manhattan distance for house proximity calculation
- Properly manages house ownership and seed ownership
- Cleans up food and updates house storage correctly

## Known Behavior
- Units prioritize eating from their own house over stealing (when they have one with food)
- Stealing only occurs when BOTH morality is very low (< 10) AND hunger is critical (<= 30)
- Seeds from stolen food still belong to the house owner, maintaining property rights
- The stealer does not gain ownership of dropped seeds
