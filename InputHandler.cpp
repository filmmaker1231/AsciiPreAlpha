#include "sdlHeader.h"
#include "UnitManager.h"
#include "InputHandler.h"
#include "Pathfinding.h"
#include "Food.h"

// Pass sdl& app as a parameter
 // Make sure to include your pathfinding header

// Initialize debounce timers
Uint32 lastUnitSpawnTime = 0;
Uint32 lastFoodSpawnTime = 0;

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

    // Delete unit or food with D + click
    if (dHeld && (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        // Try to delete a unit first
        bool deletedUnit = false;
        if (app.unitManager) {
            deletedUnit = app.unitManager->deleteUnitAt(mouseX, mouseY);
        }
        
        // If no unit was deleted, try to delete food
        if (!deletedUnit && app.foodManager) {
            app.foodManager->deleteFoodAt(mouseX, mouseY);
        }
    }







}





