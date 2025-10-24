#pragma once
#include <SDL.h>
#include "CellGrid.h"

struct sdl {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    CellGrid* cellGrid = nullptr;
    bool showCellGrid = false;
};

sdl runSdl();
void sdlDestroyWindow(sdl& app);
