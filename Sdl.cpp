#include "Sdl.h"
#include "sdlWindow.h"
#include "UnitManager.h"
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
    
    state.unitManager = new UnitManager();
    if (!state.unitManager->initializeFont("", 24)) {
        std::cerr << "Warning: Failed to initialize font for units." << std::endl;
    }
    
    return state;
}

void sdlDestroyWindow(sdl& app) {
    if (app.renderer) {
        SDL_DestroyRenderer(app.renderer);
    }
    if (app.window) {
        SDL_DestroyWindow(app.window);
    }
    if (app.cellGrid) {
        delete app.cellGrid;
    }
    if (app.unitManager) {
        delete app.unitManager;
    }
    TTF_Quit();
    SDL_Quit();
}
