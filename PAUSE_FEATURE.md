# Pause Feature

## Overview
The game now includes a pause and play functionality that allows you to pause and resume the simulation.

## How to Use
- Press **SPACE** key to toggle between pause and play states
- When paused:
  - All game logic is frozen (units stop moving, timers stop, etc.)
  - The game world continues to render so you can see the current state
  - A semi-transparent overlay with a yellow indicator box appears in the center of the screen
  
## Implementation Details
- Pause state is stored in the `sdl.isPaused` boolean flag
- SPACE key has a 200ms debounce to prevent accidental double-toggles
- Frame counter only advances when not paused, preserving accurate game timing
- All game logic (unit AI, pathfinding, hunger, fighting, etc.) is suspended during pause
- Rendering continues normally so the game world remains visible

## Controls Summary
- **SPACE**: Toggle Pause/Play
- **U + Click**: Spawn unit
- **F + Click**: Spawn food
- **C + Click**: Spawn coin
- **P + Click**: Set path for last unit
- **D + Click**: Delete unit or food
