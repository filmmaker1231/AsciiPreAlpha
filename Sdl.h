#pragma once
#include <SDL.h>
#include "CellGrid.h"

runSdl();

struct sdl {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    CellGrid* cellGrid = nullptr;
    bool showCellGrid = false;
};
