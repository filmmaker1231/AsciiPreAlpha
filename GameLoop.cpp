#include "GameLoop.h"
#include "CellGrid.h"
#include "UnitManager.h"
#include "InputHandler.h"
#include "PathClick.h"
#include "Actions.h"
#include "Unit.h"
#include "Food.h"
#include <vector>






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


		Uint32 now = SDL_GetTicks();

        // Process actions for each unit and re-queue Wander periodically if empty
		for (auto& unit : app.unitManager->getUnits()) {

			// --- HUNGER LOGIC START ---
            // Decrease hunger by 1 every 10 seconds (10000 ms)
			if (now - unit.lastHungerUpdate >= 10000) {
				if (unit.hunger > 0) {
					unit.hunger -= 1;
				}
				unit.lastHungerUpdate = now;
			}
			// Print hunger every 30 seconds (30000 ms)
			if (now - unit.lastHungerDebugPrint >= 30000) {
				std::cout << "Unit " << unit.name << " (id " << unit.id << ") hunger: " << unit.hunger << std::endl;
				unit.lastHungerDebugPrint = now;
			}
			// --- HUNGER LOGIC END ---


			// If hungry and not already seeking food, try to find food
			if (unit.hunger <= 99) {
				bool alreadySeekingFood = false;
				if (!unit.actionQueue.empty()) {
					Action current = unit.actionQueue.top();
					if (current.type == ActionType::Eat) {
						alreadySeekingFood = true;
					}
				}
				if (!alreadySeekingFood) {
					unit.tryFindAndPathToFood(*app.cellGrid, app.foodManager->getFood());
				}
			}

			

			unit.processAction(*app.cellGrid, app.foodManager->getFood());

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

		if (app.foodManager) {
			app.foodManager->renderFood(app.renderer);
			
		}

		


        SDL_RenderPresent(app.renderer);
        SDL_Delay(16); // ~60 FPS
    }
}


