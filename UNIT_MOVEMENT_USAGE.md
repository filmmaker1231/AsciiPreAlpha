# Unit Movement Speed Configuration

## Overview
Units in the game can wander around the map. The speed at which they move is controlled by the `moveDelay` property.

## Default Behavior
By default, units move with a `moveDelay` of 200 milliseconds (0.2 seconds), which means they take a step every 200ms, resulting in approximately 5 moves per second.

## Customizing Movement Speed

### Making Units Move Slower
To make a unit move slower, increase its `moveDelay` value:

```cpp
// After spawning a unit
Unit& unit = app.unitManager->getUnits()[0];
unit.moveDelay = 500;  // Unit moves every 500ms (2 moves per second)
```

### Making Units Move Faster
To make a unit move faster, decrease its `moveDelay` value:

```cpp
Unit& unit = app.unitManager->getUnits()[0];
unit.moveDelay = 100;  // Unit moves every 100ms (10 moves per second)
```

### Setting Per-Unit Speeds
You can give different units different speeds:

```cpp
// Fast scout unit
Unit& scout = app.unitManager->getUnits()[0];
scout.moveDelay = 100;  // Fast: 10 moves/sec

// Normal guard unit
Unit& guard = app.unitManager->getUnits()[1];
guard.moveDelay = 200;  // Normal: 5 moves/sec (default)

// Slow merchant unit
Unit& merchant = app.unitManager->getUnits()[2];
merchant.moveDelay = 500;  // Slow: 2 moves/sec
```

### Setting Initial Speed During Spawn
If you want to set the speed immediately when creating units, you can modify the constructor call or set it right after spawning:

```cpp
app.unitManager->spawnUnit(200, 150, "FastScout");
app.unitManager->getUnits().back().moveDelay = 100;

app.unitManager->spawnUnit(400, 300, "NormalGuard");
// Uses default 200ms delay

app.unitManager->spawnUnit(600, 450, "SlowMerchant");
app.unitManager->getUnits().back().moveDelay = 500;
```

## Recommended Values

| Speed Type | moveDelay (ms) | Moves per Second | Use Case |
|------------|---------------|------------------|----------|
| Very Fast  | 50-100        | 10-20            | Fast scouts, fleeing units |
| Fast       | 100-150       | 6-10             | Alert guards, pursuing enemies |
| Normal     | 150-250       | 4-6              | Regular units, default wandering |
| Slow       | 250-500       | 2-4              | Merchants, cautious patrols |
| Very Slow  | 500-1000      | 1-2              | Heavy units, casual wandering |

## Implementation Details

The movement delay is implemented using SDL_GetTicks() to track elapsed time between moves. The unit will only advance to the next position in its path when at least `moveDelay` milliseconds have passed since the last move.

This approach ensures smooth, framerate-independent movement that works consistently across different system speeds.
