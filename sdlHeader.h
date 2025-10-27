#pragma once
#include <SDL.h>
#include "CellGrid.h"
#include "Food.h"


class UnitManager; // Forward declaration
class FoodManager; // Forward declaration
class SeedManager; // Forward declaration
class CoinManager; // Forward declaration
class StickManager; // Forward declaration
class FiresticksManager; // Forward declaration
class ClayManager; // Forward declaration
class ShapedClayManager; // Forward declaration
class BrickManager; // Forward declaration
class DryGrassManager; // Forward declaration
class PiggyBankManager; // Forward declaration
class UnfinishedKilnManager; // Forward declaration
class KilnManager; // Forward declaration


struct sdl {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    CellGrid* cellGrid = nullptr;
    UnitManager* unitManager = nullptr;
	FoodManager* foodManager = nullptr;
	SeedManager* seedManager = nullptr;
	CoinManager* coinManager = nullptr;
	StickManager* stickManager = nullptr;
	FiresticksManager* firesticksManager = nullptr;
	ClayManager* clayManager = nullptr;
	ShapedClayManager* shapedClayManager = nullptr;
	BrickManager* brickManager = nullptr;
	DryGrassManager* dryGrassManager = nullptr;
	PiggyBankManager* piggyBankManager = nullptr;
	UnfinishedKilnManager* unfinishedKilnManager = nullptr;
	KilnManager* kilnManager = nullptr;

    bool showCellGrid = false;
	bool isPaused = false;
	
	
};

sdl runSdl();
void sdlDestroyWindow(sdl& app);
