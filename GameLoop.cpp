#include "GameLoop.h"
#include "CellGrid.h"
#include "UnitManager.h"
#include "InputHandler.h"
#include "PathClick.h"
#include "Actions.h"
#include "Unit.h"
#include "Food.h"
#include "Buildings.h"

#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>

void runMainLoop(sdl& app) {
    bool running = true;
    SDL_Event event;
    static int frameCounter = 0;
    const int HUNGER_CHECK_FRAMES = 60; // Check hunger once every 60 frames (~1 second at 60 FPS)

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        handleInput(app);
        pathClick(app);
        ++frameCounter;

        Uint32 now = SDL_GetTicks();

        // Process units
        for (auto& unit : app.unitManager->getUnits()) {



			// --- AUTO BRING FOOD TO HOUSE LOGIC ---
// Only if the unit is not already bringing food, and house is not full
			bool alreadyBringingFood = false;
			if (!unit.actionQueue.empty()) {
				Action current = unit.actionQueue.top();
				if (current.type == ActionType::BringItemToHouse && current.itemType == "food") {
					alreadyBringingFood = true;
				}
			}

			// Only try to bring food if there is food available
			if (!alreadyBringingFood && g_HouseManager && app.foodManager && !app.foodManager->getFood().empty()) {
				for (auto& house : g_HouseManager->houses) {
					if (house.ownerUnitId == unit.id &&
						house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
						if (house.hasSpace()) {
							unit.bringItemToHouse("food");
						}
						break;
					}
				}
			}





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
                std::cout << "Unit " << unit.name
                          << " (id " << unit.id << ") hunger: "
                          << unit.hunger << std::endl;
                unit.lastHungerDebugPrint = now;
            }
            // --- HUNGER LOGIC END ---



			// --- EAT FROM HOUSE LOGIC ---
			// If hunger is below 50, try to eat from house storage first
			bool tryingToEatFromHouse = false;
			if ((frameCounter % HUNGER_CHECK_FRAMES == 0) && unit.hunger < 50) {
				bool alreadyEatingFromHouse = false;
				if (!unit.actionQueue.empty()) {
					Action current = unit.actionQueue.top();
					if (current.type == ActionType::EatFromHouse) {
						alreadyEatingFromHouse = true;
					}
				}
				if (!alreadyEatingFromHouse && g_HouseManager) {
					// Check if unit has a house with food
					for (auto& house : g_HouseManager->houses) {
						if (house.ownerUnitId == unit.id &&
							house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
							if (house.hasItem("food")) {
								unit.eatFromHouse();
								tryingToEatFromHouse = true;
							}
							break;
						}
					}
				}
			}



            // If hungry and not already seeking food, try to find food from world
            // Only check this periodically to optimize performance
            // Skip if unit is trying to eat from house (hunger < 50 and house has food)
            if ((frameCounter % HUNGER_CHECK_FRAMES == 0) && unit.hunger <= 99 && !tryingToEatFromHouse) {
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










            // Process queued actions - only if there's something to process
            if (!unit.actionQueue.empty() || !unit.path.empty()) {
                unit.processAction(*app.cellGrid, app.foodManager->getFood());
            }

            // If no actions left, re-add Wander
            if (unit.actionQueue.empty()) {
                unit.addAction(Action(ActionType::Wander, 1));
            }
        }

        // --- RENDERING ---
        SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
        SDL_RenderClear(app.renderer);

        renderCellGrid(app.renderer, *app.cellGrid, app.showCellGrid);

		// --- RENDER HOUSES ---
		// --- RENDER HOUSES ---
		if (g_HouseManager) {
			SDL_SetRenderDrawColor(app.renderer, 139, 69, 19, 255); // Brown

			for (const auto& s : g_HouseManager->houses) {
				// Draw house tiles
				for (int dx = 0; dx < 3; ++dx) {
					for (int dy = 0; dy < 3; ++dy) {
						int px, py;
						app.cellGrid->gridToPixel(s.gridX + dx, s.gridY + dy, px, py);
						SDL_Rect rect = { px, py, GRID_SIZE, GRID_SIZE };
						SDL_RenderFillRect(app.renderer, &rect);

						// Draw stored item if present
						if (!s.items[dx][dy].empty()) {
							// For food, draw a more visible indicator
							if (s.items[dx][dy] == "food") {
								SDL_SetRenderDrawColor(app.renderer, 255, 255, 0, 255); // Yellow for better visibility
								// Draw a larger filled rect to make food more visible
								SDL_Rect itemRect = { px + GRID_SIZE / 6, py + GRID_SIZE / 6, 
													 (GRID_SIZE * 2) / 3, (GRID_SIZE * 2) / 3 };
								SDL_RenderFillRect(app.renderer, &itemRect);
							} else {
								// Other items - red
								SDL_SetRenderDrawColor(app.renderer, 255, 0, 0, 255);
								SDL_Rect itemRect = { px + GRID_SIZE / 4, py + GRID_SIZE / 4, 
													 GRID_SIZE / 2, GRID_SIZE / 2 };
								SDL_RenderFillRect(app.renderer, &itemRect);
							}
							SDL_SetRenderDrawColor(app.renderer, 139, 69, 19, 255); // Reset to house color
						}
					}
				}
			}
		}

        // Render units and their paths
        if (app.unitManager) {
            app.unitManager->renderUnits(app.renderer);
            app.unitManager->renderUnitPaths(app.renderer, *app.cellGrid);
        }

        

        // Render food
        if (app.foodManager) {
            app.foodManager->renderFood(app.renderer);
        }

        SDL_RenderPresent(app.renderer);
        SDL_Delay(16); // ~60 FPS
    }
}
