#include "GameLoop.h"
#include "CellGrid.h"
#include "UnitManager.h"

void runMainLoop(sdl& app) {
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
        SDL_RenderClear(app.renderer);
        renderCellGrid(app.renderer, *app.cellGrid, app.showCellGrid);
        if (app.unitManager) {
            app.unitManager->renderUnits(app.renderer);
        }
        SDL_RenderPresent(app.renderer);
        SDL_Delay(16); // ~60 FPS
    }
}
