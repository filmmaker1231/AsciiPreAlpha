# Market System Implementation

This document describes the market economy system added to the ASCII game.

## Overview
The market system allows units to buy and sell food using coins. When a unit's house becomes full, they will automatically go to the market to sell their food. Other units with coins can then buy from the market.

## Key Components

### Coin Object
- **Symbol**: `$` (displayed in gold color)
- **Spawn**: Press `C` key + click to spawn a coin
- **Storage**: Coins can be stored in houses (3x3 grid)
- **Inventory**: Units can carry coins invisibly in their inventory

### Market Building
- **Structure**: 3x3 building rendered in light tan color
- **Stalls**: Each tile in the market represents one market stand
- **Location**: A test market is placed at grid coordinates (10, 10)
- **Capacity**: Up to 9 concurrent sellers (one per stall)

## Actions

### Sell at Market (Priority 2)
**Trigger**: When a unit's house is full (no empty spaces)

**Process**:
1. Unit paths to their house
2. Picks up food from house storage
3. Paths to an empty market stall
4. Stands at stall waiting for buyers
5. Food remains visible at the stall

**Special Behaviors**:
- Seller can leave for higher priority tasks (eating, fighting)
- Seller returns to stall after completing higher priority action
- Food left at stall is still visible and available
- If seller doesn't return within 200 seconds, food becomes free for anyone

### Buy at Market (Priority 2)
**Trigger**: When unit meets ALL conditions:
- Home is not full of food
- Has at least 1 coin in house storage
- There is an active seller at the market

**Process**:
1. Unit paths to house and picks up coin (invisible inventory)
2. Paths to market stall with active seller
3. Makes transaction:
   - Gives coin to seller (placed at stall)
   - Receives food from seller
   - Debug prints: "(buyer) and (seller) have made a deal"
4. Handles food based on hunger:
   - If hunger < 50: Eats immediately (seeds still drop)
   - If hunger >= 50: Brings food home to store

### Bring Coin Home (Priority 3)
**Automatic**: Triggered when seller receives coin from sale

**Process**:
1. Seller picks up coin from stall
2. Paths back to house
3. Stores coin in house storage

## Implementation Details

### Food Handling
- Food objects are **never deleted and recreated**
- Same food instance is visually carried by units
- Food position updates to unit position when carried
- Food position updates to stall when selling
- Maintains visual continuity throughout all transfers

### Priority System
```
Priority 9: Eat (when hungry), Fight
Priority 8: BuildHouse, EatFromHouse
Priority 5: BringItemToHouse
Priority 4: BuildFarm, PlantSeed, HarvestFood
Priority 3: BringCoinToHouse
Priority 2: SellAtMarket, BuyAtMarket  ← Market actions
Priority 1: Wander (default behavior)
```

### Stall Management
Each market stall tracks:
- `stallFoodIds[3][3]` - Food being sold at each stall
- `stallSellerIds[3][3]` - Unit ID of seller at each stall
- `stallAbandonTimes[3][3]` - Timestamp for abandonment tracking

Abandonment logic:
- Timer starts when seller leaves stall
- Timer resets when seller returns to stall
- After 200 seconds (200,000 ms), food becomes free
- Stall is cleared and available for next seller

## Files Modified

### Core Classes
- `Food.h` / `Food.cpp` - Added Coin and CoinManager classes
- `Buildings.h` / `Buildings.cpp` - Added Market structure and coin storage in houses
- `Unit.h` / `Unit.cpp` - Added selling state, coin inventory, and market actions

### Game Loop
- `GameLoop.cpp` - Market rendering, auto-triggers, abandonment logic, transaction handling
- `Actions.h` - New action types: SellAtMarket, BuyAtMarket, BringCoinToHouse

### Input/Initialization
- `InputHandler.cpp` - Coin spawning with 'C' key
- `UnitManager.cpp` - Market initialization
- `Sdl.cpp` / `sdlHeader.h` - CoinManager and MarketManager setup

## Testing

### Quick Test
1. Run the game
2. Spawn food near units (F + click)
3. Watch units collect food and fill houses
4. Spawn coins in houses (C + click on brown tiles)
5. Watch market transactions occur

### Expected Console Output
```
Unit Bubby picked up food (id 1) to sell at market.
Unit Bubby is now selling at market stall (10, 10).
Unit Charles picked up coin (id 1) to buy at market.
Unit Charles (buyer) and seller (id 1) have made a deal.
Unit Bubby picked up coin (id 1) from sale to bring home.
Unit Bubby stored coin at home.
Unit Charles consumed purchased food on the spot.
```

## Visual Guide

### Colors
- **Yellow '@'**: Units
- **Yellow 'f'**: Food
- **Gold '$'**: Coins
- **Brown**: Houses (3x3)
- **Olive Green**: Farms (3x3)
- **Light Tan**: Market (3x3)

### Controls
- `U` + click: Spawn unit
- `F` + click: Spawn food
- `C` + click: Spawn coin
- `P` + click: Path unit to location
- `D` + click: Delete unit/food

## Notes

- Coins in inventory are not visually displayed (by design)
- Coins at stalls and in storage are visible as gold '$'
- Multiple units can sell at different stalls simultaneously
- System automatically manages priorities to ensure units eat when hungry
- Sellers will return to stalls after handling urgent matters
