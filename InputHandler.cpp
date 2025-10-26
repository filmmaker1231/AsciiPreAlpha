#include "sdlHeader.h"
#include "UnitManager.h"
#include "InputHandler.h"
#include "Pathfinding.h"
#include "Food.h"
#include "Buildings.h"

// Pass sdl& app as a parameter
 // Make sure to include your pathfinding header

// Initialize debounce timers
Uint32 lastUnitSpawnTime = 0;
Uint32 lastFoodSpawnTime = 0;
Uint32 lastDeleteTime = 0;

void handleInput(sdl& app) {
    const Uint8* keyState = SDL_GetKeyboardState(nullptr);
    Uint32 currentTime = SDL_GetTicks();


	bool fHeld = keyState[SDL_SCANCODE_F];
    bool uHeld = keyState[SDL_SCANCODE_U];
    bool pHeld = keyState[SDL_SCANCODE_P];
    bool dHeld = keyState[SDL_SCANCODE_D];

    int mouseX, mouseY;
    Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);

	


    
	


    // Spawn unit with U + click (with debounce)
    if (uHeld && (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        if (currentTime - lastUnitSpawnTime >= SPAWN_DEBOUNCE_MS) {
            if (app.unitManager) {
                app.unitManager->spawnUnit(mouseX, mouseY, "unit", app.cellGrid);
                lastUnitSpawnTime = currentTime;
            }
        }
    }


	if (fHeld && (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        if (currentTime - lastFoodSpawnTime >= SPAWN_DEBOUNCE_MS) {
            if (app.foodManager) {
                app.foodManager->spawnFood(mouseX, mouseY, "food");
                lastFoodSpawnTime = currentTime;
            }
        }
    }

    // Path last unit with P + click
    if (pHeld && (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        if (app.unitManager && app.cellGrid) {
            auto& units = app.unitManager->getUnits();
            if (!units.empty()) {
                // Get last placed unit
                auto& unit = const_cast<Unit&>(units.back());

                // Convert unit and mouse to grid coordinates
                int unitGridX, unitGridY, mouseGridX, mouseGridY;
                app.cellGrid->pixelToGrid(unit.x, unit.y, unitGridX, unitGridY);
                app.cellGrid->pixelToGrid(mouseX, mouseY, mouseGridX, mouseGridY);

                // Find path
                auto path = aStarFindPath(unitGridX, unitGridY, mouseGridX, mouseGridY, *app.cellGrid);

                // Assign path to unit
                unit.path = path;
            }
        }
    }

    // Delete unit or food with D + click (with debounce)
    if (dHeld && (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        if (currentTime - lastDeleteTime >= DELETE_DEBOUNCE_MS) {
            // Try to delete a unit first
            bool deletedUnit = false;
            if (app.unitManager) {
                // First, find if there's a unit at this location and get its carried items
            int carriedFoodId = -1;
            int carriedSeedId = -1;
            const int clickRadius = 20;
            for (const auto& unit : app.unitManager->getUnits()) {
                if (mouseX >= unit.x - clickRadius && mouseX <= unit.x + clickRadius &&
                    mouseY >= unit.y - clickRadius && mouseY <= unit.y + clickRadius) {
                    carriedFoodId = unit.carriedFoodId;
                    carriedSeedId = unit.carriedSeedId;
                    break;
                }
            }
            
            // Delete the unit
            deletedUnit = app.unitManager->deleteUnitAt(mouseX, mouseY);
            
            // Clear carried items from food/seed managers
            if (deletedUnit) {
                if (carriedFoodId != -1 && app.foodManager) {
                    for (auto& foodItem : app.foodManager->getFood()) {
                        if (foodItem.foodId == carriedFoodId) {
                            foodItem.carriedByUnitId = -1;
                            break;
                        }
                    }
                }
                if (carriedSeedId != -1 && app.seedManager) {
                    for (auto& seedItem : app.seedManager->getSeeds()) {
                        if (seedItem.seedId == carriedSeedId) {
                            seedItem.carriedByUnitId = -1;
                            break;
                        }
                    }
                }
            }
        }
        
        // If no unit was deleted, try to delete food
        if (!deletedUnit && app.foodManager) {
            // First, find if there's food at this location and get its ID
            int deletedFoodId = -1;
            const int clickRadius = 20;
            for (const auto& foodItem : app.foodManager->getFood()) {
                if (mouseX >= foodItem.x - clickRadius && mouseX <= foodItem.x + clickRadius &&
                    mouseY >= foodItem.y - clickRadius && mouseY <= foodItem.y + clickRadius) {
                    deletedFoodId = foodItem.foodId;
                    break;
                }
            }
            
            // Delete the food
            if (app.foodManager->deleteFoodAt(mouseX, mouseY)) {
                if (deletedFoodId != -1) {
                    // Clear any unit carrying this food
                    if (app.unitManager) {
                        for (auto& unit : app.unitManager->getUnits()) {
                            if (unit.carriedFoodId == deletedFoodId) {
                                unit.carriedFoodId = -1;
                            }
                        }
                    }
                    
                    // Remove food from any house storage
                    if (g_HouseManager) {
                        for (auto& house : g_HouseManager->houses) {
                            house.removeFoodById(deletedFoodId);
                        }
                    }
                }
            }
        }
        
        // Update debounce timer
        lastDeleteTime = currentTime;
        }
    }







}





