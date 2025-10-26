#pragma once
#include <SDL.h>
#include "CellGrid.h"
#include "Food.h"


class UnitManager; // Forward declaration
class FoodManager; // Forward declaration


struct sdl {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    CellGrid* cellGrid = nullptr;
    UnitManager* unitManager = nullptr;
	FoodManager* foodManager = nullptr;

    bool showCellGrid = false;
	
	
};

sdl runSdl();
void sdlDestroyWindow(sdl& app);
