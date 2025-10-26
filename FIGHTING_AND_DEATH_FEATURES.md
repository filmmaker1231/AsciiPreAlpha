# Fighting and Death Features

## Overview
This document describes the newly implemented fighting mechanics and unit death system in the game.

## Death System

### Death Conditions
A unit will be permanently deleted from the game when **EITHER** of the following conditions is met:
1. Unit's hunger reaches 0 or below (`hunger <= 0`)
2. Unit's health reaches 0 or below (`health <= 0`)

### Death Behavior
1. **Console Message**: When a unit dies, a message is printed:
   ```
   Unit UnitName (id X) has died: hunger reached 0
   ```
   or
   ```
   Unit UnitName (id X) has died: health reached 0
   ```

2. **Cleanup**: Upon death, the following cleanup occurs:
   - Any food being carried is released (marked as not carried)
   - Any seeds being carried are released (marked as not carried)
   - All references to this unit in other units are cleared:
     - `stolenFromByUnitId` is cleared in other units
     - `fightingTargetId` is cleared in other units
   - The unit is removed from the game world

### Implementation Details
- Death check occurs at the end of each game loop iteration, after all units have processed their actions
- Located in `GameLoop.cpp` after the main unit processing loop
- Uses iterator-based deletion to safely remove units while iterating

## Health System

### Basic Mechanics
- **Starting Value**: Units start with health at 100
- **Range**: Health ranges from 0 (minimum) to no defined maximum
- **Current Damage Sources**: Fighting (see below)

### Implementation Details
- Health field already existed in Unit class: `int health`
- Health is initialized to 100 in the Unit constructor
- Health can be decreased by combat or other game mechanics

## Fighting Mechanics

### When Units Fight
A unit will engage in combat when **ALL** of the following conditions are met:
1. Another unit stole food from their house
2. The thief is within 5 tiles distance (Manhattan distance)
3. The victim is not currently clamped in another fight

### Fighting Behavior

#### 1. Detection and Initiation
- **Theft Tracking**: When food is stolen, the victim's `stolenFromByUnitId` is set to the thief's ID
- **Range Check**: Every frame, victims check if their thief is within 5 tiles
- **Action Priority**: Fighting has priority 9 (highest priority, same as Eat from world)

#### 2. Pursuit Phase
- **Path Updates**: The victim's path to the thief is updated every 30 frames to track moving thieves
- **Speed Boost**: Victim moves faster than normal:
  - Normal speed: `moveDelay = 50` (50ms between moves)
  - Chase speed: `moveDelay = 30` (30ms between moves)
- **Continuous Tracking**: Path is recalculated using A* pathfinding to ensure optimal pursuit

#### 3. Combat Phase
When units become adjacent (distance <= 1):
1. **Clamping**: Both units are locked in position for 2 seconds
   - `isClamped = true` for both units
   - `fightStartTime` is set to current time for both units
2. **Damage**: The thief takes 10 health damage immediately
3. **Message**: Console output:
   ```
   VictimName has hit ThiefName for 10 damage for stealing from them!
   ```
4. **Tracking Cleared**: The victim's `stolenFromByUnitId` is cleared after landing the hit

#### 4. Clamp Duration
- **Duration**: 2 seconds (2000 milliseconds)
- **Movement Prevention**: Both units have their paths cleared every frame while clamped
- **Auto-Release**: After 2 seconds, both units are automatically unclamped
- **Speed Restoration**: Normal speed (50ms) is restored when unclamped

#### 5. Fight Termination
Fighting ends when any of the following occurs:
1. The victim successfully hits the thief (clamp begins)
2. The thief moves beyond 5 tiles from the victim
3. The thief is deleted (dies or removed from game)
4. The victim is deleted (dies or removed from game)

In cases 2-4, the victim's speed is immediately restored to normal (50ms).

### Messages
When combat occurs, the following message is printed to console:
```
VictimName has hit ThiefName for 10 damage for stealing from them!
```

Additional tracking message when theft is detected:
```
Victim VictimName (id X) now knows that ThiefName (id Y) stole from them
```

### Technical Implementation Details

#### New Fields in Unit Class
- `int stolenFromByUnitId = -1`: ID of unit who stole from this unit, -1 if none
- `int justStoleFromUnitId = -1`: ID of unit this unit just stole from (cleared after processing)
- `int fightingTargetId = -1`: ID of unit currently being fought, -1 if none
- `Uint32 fightStartTime = 0`: Time when fight started (for 2-second clamp)
- `bool isClamped = false`: Whether unit is clamped during fight

#### Action Priority System Update
- Priority 1: Wander (lowest)
- Priority 3: CollectSeed
- Priority 4: BuildFarm, PlantSeed, HarvestFood
- Priority 5: BringItemToHouse (food)
- Priority 8: BuildHouse, EatFromHouse, StealFood
- Priority 9: Eat (from world), **Fight** (new)

Higher priority actions interrupt lower priority actions.

#### Files Modified
1. **Unit.h**: Added fighting and death tracking fields
2. **Actions.h**: Added `Fight` action type
3. **GameLoop.cpp**: 
   - Added fight detection and combat logic
   - Added unit deletion logic for hunger/health <= 0
   - Added theft victim tracking
   - Included `Pathfinding.h` for A* path updates
4. **Unit.cpp**: 
   - Added `ActionType::Fight` case in `processAction()`
   - Modified `StealFood` to record `justStoleFromUnitId`

## Testing Scenarios

### Scenario 1: Unit Death by Hunger
1. Spawn a unit with U+Click
2. Do not provide any food
3. Wait for hunger to decrease to 0 (takes ~50 seconds at 1 hunger per 500ms)
4. **Expected Result**: Unit is deleted with message "Unit [Name] (id X) has died: hunger reached 0"

### Scenario 2: Unit Death by Health (via Combat)
1. Spawn Unit A with U+Click
2. Spawn food near Unit A with F+Click
3. Wait for Unit A to build house and store food
4. Spawn Unit B far from food sources
5. Wait for Unit B to steal from Unit A (requires low morality and hunger)
6. Wait for Unit A to attack Unit B (within 5 tiles)
7. Let Unit B be attacked 10 times (10 damage each = 100 total)
8. **Expected Result**: Unit B is deleted with message "Unit [Name] (id X) has died: health reached 0"

### Scenario 3: Fighting and Pursuit
1. Spawn Unit A (Victim) and Unit B (Thief) with U+Click
2. Give Unit A a house with food
3. Make Unit B steal from Unit A's house (low morality + hunger)
4. Position units within 5 tiles after theft
5. **Expected Results**:
   - Message: "Victim [A] (id X) now knows that [B] (id Y) stole from them"
   - Unit A starts chasing Unit B at faster speed
   - Path updates every ~0.5 seconds to track Unit B
   - When adjacent: Both units freeze for 2 seconds
   - Message: "[A] has hit [B] for 10 damage for stealing from them!"
   - After 2 seconds: Both units resume normal behavior

### Scenario 4: Thief Escapes
1. Set up fight as in Scenario 3
2. Have the thief move beyond 5 tiles before being caught
3. **Expected Result**: 
   - Victim stops chasing
   - Victim's speed returns to normal (50ms)
   - No combat occurs

### Scenario 5: Multiple Victims
1. Spawn three units: Unit A, Unit B (has food), Unit C (has food)
2. Make Unit A steal from both Unit B and Unit C
3. Position all units within 5 tiles
4. **Expected Results**:
   - Both Unit B and Unit C will independently chase Unit A
   - Whichever victim reaches Unit A first will engage in combat
   - The other victim may also fight if still within range

## Known Behavior
- Fighting only occurs for food theft from houses, not for other actions
- A unit can only track one thief at a time (the most recent)
- Multiple victims can chase the same thief simultaneously
- Clamping prevents ALL movement, including action-based movement
- Speed boost is only applied during active chase, not while clamped
- Death cleanup ensures no dangling references to deleted units
