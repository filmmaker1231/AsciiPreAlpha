#pragma once
#include "sdlHeader.h"
#include "UnitManager.h"

void handleInput(sdl& app);

// Debounce timers for spawning (in milliseconds)
extern Uint32 lastUnitSpawnTime;
extern Uint32 lastFoodSpawnTime;
const Uint32 SPAWN_DEBOUNCE_MS = 300; // 300ms between spawns

