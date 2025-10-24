#include "Sdl.h"
#include "sdlWindow.h"
#include <SDL_ttf.h>
#include <iostream>

sdl runSdl() {
    sdl state;
    state.renderer = nullptr;
    state.window = startSdlWindow(state.renderer);
    if (!state.window || !state.renderer) {
        std::cerr << "Failed to create SDL window or renderer." << std::endl;
        return state;
    }

    int gridWidth, gridHeight;
    SDL_GetWindowSize(state.window, &gridWidth, &gridHeight);

    state.cellGrid = new CellGrid(gridWidth, gridHeight);
    state.showCellGrid = false;
    return state;
}

void sdlDestroyWindow(sdl& app) {
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    delete app.cellGrid;
    TTF_Quit();
    SDL_Quit();
}
