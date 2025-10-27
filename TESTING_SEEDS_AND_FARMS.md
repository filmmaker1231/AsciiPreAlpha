# Testing Seeds and Farming System

## Overview
This document describes how to test the newly implemented seeds and farming system.

## Controls
- **U + Click**: Spawn a unit
- **F + Click**: Spawn food
- **P + Click**: Path the last spawned unit to clicked location

## Testing Sequence

### 1. Test Seed Dropping
1. Spawn a unit with U+Click
2. Spawn food near the unit with F+Click
3. Wait for the unit to build a house (happens automatically)
4. The unit will automatically collect the food and bring it home
5. When the unit's hunger drops below 50, it will eat from house storage
6. **Expected Result**: When eating, 1-2 seeds (with '.' symbol) should drop at the eating location
   - 85% chance: 1 seed drops
   - 15% chance: 2 seeds drop
7. Seeds dropped inside the house are owned by the house owner
8. Seeds dropped outside are unowned

### 2. Test Seed Collection (Priority 3)
1. After seeds have been dropped, the unit should automatically collect them
2. **Expected Result**: Unit paths to the seed, picks it up (seed follows unit), and brings it to the house
3. Seeds in the house are stored in the 3x3 grid (same as food but cannot stack on same tile)

### 3. Test Farm Building (Priority 4)
1. Once the unit has at least 1 seed in the house, it should automatically build a farm
2. **Expected Result**: A brownish-green 3x3 farm appears exactly 1 space away from the house
3. The farm should be linked to the unit (owned by the same unit)

### 4. Test Seed Planting (Priority 4)
1. After farm is built, the unit should automatically plant seeds from the house
2. **Expected Result**: Unit picks up seed from house, walks to farm, and plants it
3. Each farm tile can hold 1 seed (max 9 seeds per farm)
4. Seeds planted in farm are visually represented with '.' symbol

### 5. Test Seed Growth
1. After planting, seeds should grow into food after 10 seconds
2. **Expected Result**: After 10 seconds, the seed changes into a food object ('f' symbol)
3. This food is owned exclusively by the farm owner

### 6. Test Food Harvesting (Priority 4)
1. Once food has grown in the farm, the unit should automatically harvest it
2. **Expected Result**: Unit walks to farm, picks up the food, and brings it to the house
3. The food is stored in the house like normal food
4. The unit can then eat this food when hungry

## Priority System
The action priorities are:
- Priority 1: Wander (lowest)
- Priority 3: CollectSeed
- Priority 4: BuildFarm, PlantSeed, HarvestFood
- Priority 5: BringItemToHouse (food)
- Priority 8: BuildHouse, EatFromHouse
- Priority 9: Eat (from world)

Higher priority actions interrupt lower priority actions.

## Expected Behavior
1. Units will prioritize eating from house storage when hungry
2. Units will collect food from the world when available
3. Seeds drop when eating (either from world or from house)
4. Units collect seeds when they appear (if house has space)
5. Units build farms when they have seeds
6. Units plant seeds in farms
7. Seeds grow into food after 10 seconds
8. Units harvest grown food and store it in house
9. This creates a sustainable food production cycle

## Known Limitations
- Farm must be exactly 1 space away from house (4 cardinal directions checked)
- Only 1 item (food OR seed) per tile in house storage
- Farm can hold max 9 plants (3x3 grid)
- Growth time is fixed at 10 seconds
