#include "GameLoop.h"
#include "CellGrid.h"
#include "UnitManager.h"
#include "InputHandler.h"
#include "PathClick.h"
#include "Actions.h"
#include "Unit.h"

void runMainLoop(sdl& app) {
    bool running = true;
    SDL_Event event;
    static int frameCounter = 0;
    const int WANDER_READD_FRAMES = 4; // try re-adding Wander once every 4 frames

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        handleInput(app);

        pathClick(app);

        ++frameCounter;

        // Process actions for each unit and re-queue Wander periodically if empty
		for (auto& unit : app.unitManager->getUnits()) {
			unit.processAction(*app.cellGrid);

			// Only re-add Wander if the queue is empty (not every N frames)
			if (unit.actionQueue.empty()) {
				unit.addAction(Action(ActionType::Wander, 1));
			}
		}

        SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
        SDL_RenderClear(app.renderer);
        renderCellGrid(app.renderer, *app.cellGrid, app.showCellGrid);
        if (app.unitManager) {
            app.unitManager->renderUnits(app.renderer);
            app.unitManager->renderUnitPaths(app.renderer, *app.cellGrid);
        }
        SDL_RenderPresent(app.renderer);
        SDL_Delay(16); // ~60 FPS
    }
}