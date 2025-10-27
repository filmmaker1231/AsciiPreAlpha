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

            // If hungry and not already seeking food, try to find food
            // Only check this periodically to optimize performance
            if ((frameCounter % HUNGER_CHECK_FRAMES == 0) && unit.hunger <= 99) {
                bool alreadySeekingFood = false;
                if (!unit.actionQueue.empty()) {
                    Action current = unit.actionQueue.top();
                    if (current.type == ActionType::Eat || current.type == ActionType::BringFoodToHouse) {
                        alreadySeekingFood = true;
                    }
                }
                if (!alreadySeekingFood) {
                    // If hunger below 50, try to eat from house
                    if (unit.hunger < 50) {
                        unit.tryEatFromHouse();
                    } else {
                        // Otherwise, try to find food to bring home
                        unit.tryFindAndPathToFood(*app.cellGrid, app.foodManager->getFood());
                    }
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

        // Render units and their paths
        if (app.unitManager) {
            app.unitManager->renderUnits(app.renderer);
            app.unitManager->renderUnitPaths(app.renderer, *app.cellGrid);
        }

        // --- RENDER HOUSES ---
        if (g_HouseManager) {
            SDL_SetRenderDrawColor(app.renderer, 139, 69, 19, 255); // Brown

            for (const auto& s : g_HouseManager->houses) {
                for (int dx = 0; dx < 3; ++dx) {
                    for (int dy = 0; dy < 3; ++dy) {
                        int px, py;
                        app.cellGrid->gridToPixel(s.gridX + dx, s.gridY + dy, px, py);
                        SDL_Rect rect = { px, py, GRID_SIZE, GRID_SIZE };
                        SDL_RenderFillRect(app.renderer, &rect);
                    }
                }
                
                // Render food items in house
                if (!s.foodIds.empty() && app.foodManager) {
                    // Show food symbols for each stored food item
                    int foodCount = static_cast<int>(s.foodIds.size());
                    int idx = 0;
                    for (int dy = 0; dy < 3 && idx < foodCount; ++dy) {
                        for (int dx = 0; dx < 3 && idx < foodCount; ++dx) {
                            int px, py;
                            app.cellGrid->gridToPixel(s.gridX + dx, s.gridY + dy, px, py);
                            app.foodManager->renderFoodSymbol(app.renderer, px, py);
                            idx++;
                        }
                    }
                }
                
                // Render coin count in top-left corner of house
                if (s.coins > 0 && app.foodManager) {
                    int px, py;
                    app.cellGrid->gridToPixel(s.gridX, s.gridY, px, py);
                    app.foodManager->renderCoinCount(app.renderer, px, py, s.coins);
                }
            }
        }

        // --- RENDER MARKETS ---
        if (g_MarketManager) {
            SDL_SetRenderDrawColor(app.renderer, 0, 128, 255, 255); // Blue for markets

            for (const auto& market : g_MarketManager->markets) {
                for (int dx = 0; dx < 3; ++dx) {
                    for (int dy = 0; dy < 3; ++dy) {
                        int px, py;
                        app.cellGrid->gridToPixel(market.gridX + dx, market.gridY + dy, px, py);
                        SDL_Rect rect = { px, py, GRID_SIZE, GRID_SIZE };
                        SDL_RenderFillRect(app.renderer, &rect);
                    }
                }
                
                // Render market info (stock and price)
                if (app.foodManager) {
                    int px, py;
                    app.cellGrid->gridToPixel(market.gridX, market.gridY, px, py);
                    // Show food stock in top-left
                    if (market.foodStock > 0) {
                        app.foodManager->renderCoinCount(app.renderer, px, py + 20, market.foodStock);
                    }
                }
            }
        }

        // Render food
        if (app.foodManager) {
            app.foodManager->renderFood(app.renderer);
        }

        SDL_RenderPresent(app.renderer);
        SDL_Delay(16); // ~60 FPS
    }
}
