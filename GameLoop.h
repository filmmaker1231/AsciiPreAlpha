#pragma once
#include <SDL.h>
#include "CellGrid.h"

// Main event loop
void runMainLoop(SDL_Window* window, SDL_Renderer* renderer, CellGrid& cellGrid, bool& showCellGrid);
