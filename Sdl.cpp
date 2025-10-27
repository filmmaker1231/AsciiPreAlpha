#include "sdlHeader.h"
#include "sdlWindow.h"
#include "UnitManager.h"
#include <SDL_ttf.h>
#include <iostream>
#include "Food.h"
#include "Buildings.h"


sdl runSdl() {
    sdl state;
    state.renderer = nullptr;
    state.window = startSdlWindow(state.renderer);
    if (!state.window || !state.renderer) {
        std::cerr << "Failed to create SDL window or renderer." << std::endl;
        return state;
    }

    int gridWidth, gridHeight;
    SDL_GetWindowSize(state.window, &gridWidth, &gridHeight);

    state.cellGrid = new CellGrid(gridWidth, gridHeight);
    state.showCellGrid = false;
    
    state.unitManager = new UnitManager();
    if (!state.unitManager->initializeFont(nullptr, 24)) {
        std::cerr << "Warning: Failed to initialize font for units." << std::endl;
    }

	state.foodManager = new FoodManager();
	if (!state.foodManager->initializeFont(nullptr, 24)) {
		std::cerr << "Warning: Failed to initialize font for units." << std::endl;
	}

	state.seedManager = new SeedManager();
	if (!state.seedManager->initializeFont(nullptr, 24)) {
		std::cerr << "Warning: Failed to initialize font for seeds." << std::endl;
	}

	state.coinManager = new CoinManager();
	if (!state.coinManager->initializeFont(nullptr, 24)) {
		std::cerr << "Warning: Failed to initialize font for coins." << std::endl;
	}
    
    // Initialize the global house manager
    g_HouseManager = new HouseManager();
    
    // Initialize the global farm manager
    g_FarmManager = new FarmManager();
    
    // Initialize the global market manager
    g_MarketManager = new MarketManager();
    
    return state;
}

void sdlDestroyWindow(sdl& app) {
    if (app.renderer) {
        SDL_DestroyRenderer(app.renderer);
    }
    if (app.window) {
        SDL_DestroyWindow(app.window);
    }
    if (app.cellGrid) {
        delete app.cellGrid;
    }
    if (app.unitManager) {
        delete app.unitManager;
    }
    if (app.foodManager) {
        delete app.foodManager;
    }
    if (app.seedManager) {
        delete app.seedManager;
    }
    if (app.coinManager) {
        delete app.coinManager;
    }
    if (g_HouseManager) {
        delete g_HouseManager;
        g_HouseManager = nullptr;
    }
    if (g_FarmManager) {
        delete g_FarmManager;
        g_FarmManager = nullptr;
    }
    if (g_MarketManager) {
        delete g_MarketManager;
        g_MarketManager = nullptr;
    }
    TTF_Quit();
    SDL_Quit();
}
