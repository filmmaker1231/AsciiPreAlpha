#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include "sdlWindow.h"
#include "CellGrid.h"
#include "GameLoop.h"
#include "sdlHeader.h"
#include "UnitManager.h"

int main() {
    sdl app = runSdl();
    if (!app.window || !app.renderer || !app.cellGrid) {
        return 1;
    }

    // Correct call to initialize units
    initializeGameUnits(app.unitManager, app.cellGrid);
    
    // Initialize default world items
    initializeWorldItems(app);

    runMainLoop(app);

    sdlDestroyWindow(app);

    return 0;
}