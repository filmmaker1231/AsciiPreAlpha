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

// Ensure this is defined somewhere in Buildings.cpp:
// StockpileManager* StockpileManager = nullptr;

void runMainLoop(sdl& app) {
    bool running = true;
    SDL_Event event;
    static int frameCounter = 0;
    const int WANDER_READD_FRAMES = 4; // Try re-adding Wander once every 4 frames

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

            // Process queued actions
            unit.processAction(*app.cellGrid, app.foodManager->getFood());

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

        // --- RENDER STOCKPILES ---
        if (StockpileManager) {
            SDL_SetRenderDrawColor(app.renderer, 139, 69, 19, 255); // Brown

            for (const auto& s : StockpileManager->stockpiles) {
                for (int dx = 0; dx < 3; ++dx) {
                    for (int dy = 0; dy < 3; ++dy) {
                        int px, py;
                        app.cellGrid->gridToPixel(s.gridX + dx, s.gridY + dy, px, py);
                        SDL_Rect rect = { px, py, GRID_SIZE, GRID_SIZE };
                        SDL_RenderFillRect(app.renderer, &rect);
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
