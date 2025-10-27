# Market and Trading System Documentation

## Overview
This document describes the market and coin trading system implemented in the game, along with bug fixes for the food collection system.

## Bug Fixes

### 1. Unit Getting Stuck After Food Pickup
**Problem**: Units would pick up food and then get stuck because the path was cleared but not immediately recreated.

**Location**: `Unit.cpp`, `BringFoodToHouse` action (around line 161)

**Fix**: 
- After picking up food, immediately create a path to the house
- If pathfinding fails, the unit drops the food and cancels the action
- This prevents the unit from being stuck with no path and an active action

**Before**:
```cpp
// Pick up the food
carryingFoodId = it->foodId;
foods.erase(it);
path.clear(); // Path cleared, nothing else happens!
```

**After**:
```cpp
// Pick up the food
carryingFoodId = it->foodId;
foods.erase(it);
path.clear();
auto newPath = aStarFindPath(gridX, gridY, houseGridX, houseGridY, cellGrid);
if (!newPath.empty()) {
    path = newPath;
} else {
    // Can't reach house - drop food and give up
    carryingFoodId = -1;
    actionQueue.pop();
}
```

### 2. Pathfinding Failure Handling
**Problem**: When pathfinding failed in phase 2 of `BringFoodToHouse`, the action would remain active but the unit would have no path, causing an infinite loop.

**Fix**: If pathfinding fails when carrying food to the house, the unit now:
- Drops the food
- Cancels the action
- Can proceed to other tasks

## New Features

### Coin System
- Each house now has a `coins` field that tracks the unit's wealth
- Houses start with 10 coins
- Coins are gained by selling food at markets
- Coins are spent when buying food from markets
- Coins are displayed in the top-left corner of houses (gold colored text)

**Code Location**: `Buildings.h`
```cpp
struct House {
    int ownerUnitId;
    int gridX, gridY;
    std::vector<int> foodIds;
    int coins; // NEW: Coin storage
    House(int ownerId, int x, int y)
        : ownerUnitId(ownerId), gridX(x), gridY(y), coins(10) {}
};
```

### Market System
- Markets are 3x3 buildings (rendered in blue)
- Markets have:
  - `foodStock`: Amount of food available for purchase
  - `coins`: Amount of coins the market has
  - `foodPrice`: Price per food unit (default: 3 coins)
- Markets are created in the center of the map when the game initializes
- Multiple markets can exist

**Code Location**: `Buildings.h`
```cpp
struct Market {
    int gridX, gridY;
    int foodStock;
    int coins;
    int foodPrice;
    Market(int x, int y, int stock = 5, int startCoins = 50, int price = 3)
        : gridX(x), gridY(y), foodStock(stock), coins(startCoins), foodPrice(price) {}
};
```

### Trading Actions

#### SellFoodAtMarket Action
**Priority**: 5 (medium)

**Behavior**:
1. Phase 1: Unit goes to their house and picks up one food item
2. Phase 2: Unit travels to the nearest market
3. Phase 3: Unit sells the food at the market
   - Market gains 1 food stock
   - Market loses coins equal to foodPrice
   - Unit's house gains coins equal to foodPrice

**Trigger**: Automatically triggered when a unit has more than 2 food items in their house

**Code Location**: `Unit.cpp`, around line 205

#### BuyFoodAtMarket Action
**Priority**: 7 (higher priority)

**Behavior**:
1. Unit travels to the nearest market
2. Unit attempts to buy food:
   - Checks if market has stock
   - Checks if unit's house has enough coins
3. If successful:
   - House loses coins equal to foodPrice
   - Market loses 1 food stock
   - Market gains coins equal to foodPrice
   - Unit carries the food home
4. Unit stores food in house (transitions to BringFoodToHouse action)

**Trigger**: Automatically triggered when a unit has less than 1 food item in their house and at least 3 coins

**Code Location**: `Unit.cpp`, around line 279

### Automatic Trading Logic
Units will automatically trade based on their inventory:

**In GameLoop.cpp** (checked every 300 frames = ~5 seconds):
- If house has > 2 food items → Unit will sell food at market
- If house has < 1 food item and >= 3 coins → Unit will buy food at market

This creates a self-sustaining economy where units:
1. Gather food from the world
2. Sell excess food for coins
3. Buy food when running low (if they have coins)
4. Prioritize eating over trading when hungry

### Rendering

#### Coins in Houses
- Displayed in gold color at the top-left corner of each house
- Format: "{count}c" (e.g., "10c")
- Uses FoodManager's font rendering system

**Code Location**: `GameLoop.cpp`, around line 131

#### Markets
- Rendered as blue 3x3 buildings
- Food stock displayed below the market (format: "{stock}")
- Distinguishable from houses (brown) and other buildings

**Code Location**: `GameLoop.cpp`, around line 134-158

## Testing

Three test files verify the system works correctly:

### 1. test_simulation.cpp
- Tests the original bug with units getting stuck
- Simulates the BringFoodToHouse action
- Verifies the bug fix works

### 2. test_market_system.cpp
- Tests selling food at market
- Tests buying food at market
- Tests insufficient coins scenario
- Tests out of stock scenario
- Tests multiple transactions

### 3. test_complete_system.cpp
- Comprehensive integration test
- Verifies both bug fixes
- Tests complete market cycle
- Tests edge cases

All tests pass successfully!

## Usage Examples

### Creating a Market
```cpp
// In UnitManager.cpp or initialization code
if (g_MarketManager) {
    g_MarketManager->addMarket(Market(gridX, gridY, 10, 100, 3));
    // Creates market with 10 stock, 100 coins, price of 3
}
```

### Manually Triggering Trading
```cpp
// Make a unit sell food
unit.addAction(Action(ActionType::SellFoodAtMarket, 5));

// Make a unit buy food
unit.addAction(Action(ActionType::BuyFoodAtMarket, 7));
```

### Checking Market Status
```cpp
if (g_MarketManager) {
    for (const auto& market : g_MarketManager->markets) {
        std::cout << "Market at (" << market.gridX << "," << market.gridY << ")\n";
        std::cout << "  Stock: " << market.foodStock << "\n";
        std::cout << "  Coins: " << market.coins << "\n";
        std::cout << "  Price: " << market.foodPrice << "\n";
    }
}
```

## Future Enhancements

Potential improvements:
1. Dynamic pricing based on supply/demand
2. Multiple types of goods (not just food)
3. Trade routes between markets
4. Market upgrades (capacity, better prices)
5. Competition between units for resources
6. Market events (sales, shortages)
7. Unit specializations (farmers, merchants, etc.)

## Configuration

### Market Parameters
- **Default stock**: 5 food items
- **Default coins**: 50 coins
- **Default price**: 3 coins per food

### Trading Thresholds
- **Sell trigger**: House has > 2 food items
- **Buy trigger**: House has < 1 food item AND >= 3 coins
- **Check frequency**: Every 300 frames (~5 seconds at 60 FPS)

These values can be adjusted in the respective source files.
