#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include "sdlWindow.h"
#include "CellGrid.h"
#include "GameLoop.h"
#include "Sdl.h"

int main() {
    sdl app = runSdl();
    if (!app.window || !app.renderer || !app.cellGrid) {
        return 1;
    }

    runMainLoop(app.window, app.renderer, *app.cellGrid, app.showCellGrid);

    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    delete app.cellGrid;
    TTF_Quit();
    SDL_Quit();

    return 0;
}
