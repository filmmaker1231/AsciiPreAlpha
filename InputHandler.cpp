#include "CellGrid.h"
#include "Sdl.h"
#include "UnitManager.h"
#include "InputHandler.h"
#include "Pathfinding.h"
#include "Tiles.h"

// Pass sdl& app as a parameter
 // Make sure to include your pathfinding header

void handleInput(sdl& app) {
    const Uint8* keyState = SDL_GetKeyboardState(nullptr);


	bool fHeld = keyState[SDL_SCANCODE_F];
    bool uHeld = keyState[SDL_SCANCODE_U];
    bool pHeld = keyState[SDL_SCANCODE_P];

    int mouseX, mouseY;
    Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);

	


    
	// Spawn food
	if (fHeld && (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))) {
		if (app.foodManager) {
			app.foodManager->spawnFood(mouseX, mouseY, "food");
		}
	}


    // Spawn unit with U + click
    if (uHeld && (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        if (app.unitManager) {
            app.unitManager->spawnUnit(mouseX, mouseY, "unit");
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







}





