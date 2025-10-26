#include "UnitManager.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <random>
#include "CellGrid.h"
#include "Buildings.h"
#include "Unit.h"

UnitManager::UnitManager() : font(nullptr) {
}

UnitManager::~UnitManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool UnitManager::initializeFont(const char* fontPath, int fontSize) {
    // Try provided font path first (skip if nullptr or empty)
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) {
            return true;
        }
    }
    
    // Try to load a default system font
    // On Windows, try Arial
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        // On Linux, try DejaVu Sans
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void UnitManager::spawnUnit(int x, int y, const std::string& name, CellGrid* cellGrid) {
    static int nextId = 1; // Static to ensure unique IDs
    units.emplace_back(x, y, '@', name, 100, nextId++);
	Unit& unit = units.back();
    unit.hunger = 100;
	unit.lastHungerUpdate = SDL_GetTicks();
	unit.lastHungerDebugPrint = SDL_GetTicks();
    std::cout << "Spawned unit '" << name << "' at (" << x << ", " << y << ") with id " << (nextId-1) << std::endl;

	// Generate random house location
	if (cellGrid) {
		int gridWidth = cellGrid->getWidthInCells();
		int gridHeight = cellGrid->getHeightInCells();

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distX(0, gridWidth - 3);
		std::uniform_int_distribution<> distY(0, gridHeight - 3);

		int randomX = distX(gen);
		int randomY = distY(gen);

		unit.houseGridX = randomX;
		unit.houseGridY = randomY;
		unit.addAction(Action(ActionType::BuildHouse, 8));
	}
	unit.addAction(Action(ActionType::BuildHouse, 8));



}


void UnitManager::renderUnits(SDL_Renderer* renderer) {
    if (!font) {
        return;
    }
    
    SDL_Color color = {255, 255, 0, 255}; // White color
    
    for (const auto& unit : units) {
        // Create surface with the unit symbol
        std::string symbolStr(1, unit.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) {
            continue;
        }
        
        // Create texture from surface
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {unit.x, unit.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        
        SDL_FreeSurface(surface);
    }
}

void initializeGameUnits(UnitManager* unitManager, CellGrid* cellGrid) {
    if (!unitManager || !cellGrid) {
        return;
    }

    // Spawn a few sample units
    unitManager->spawnUnit(200, 150, "Player", cellGrid);
    unitManager->spawnUnit(400, 300, "Guard", cellGrid);
    unitManager->spawnUnit(600, 450, "Merchant", cellGrid);

    // Set moveDelay for all units after spawning
    for (auto& unit : unitManager->getUnits()) {
        unit.moveDelay = 50;
    }
}

void UnitManager::renderUnitPaths(SDL_Renderer* renderer, const CellGrid& cellGrid) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 128); // Green, semi-transparent

    int cellWidth = cellGrid.getWidthInPixels() / cellGrid.getWidthInCells();
    int cellHeight = cellGrid.getHeightInPixels() / cellGrid.getHeightInCells();

    for (const auto& unit : units) {
        for (const auto& cell : unit.path) {
            int px, py;
            cellGrid.gridToPixel(cell.first, cell.second, px, py);
            SDL_Rect rect = { px, py, cellWidth, cellHeight };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}


std::vector<Unit>& UnitManager::getUnits() {
    return units;
}

const std::vector<Unit>& UnitManager::getUnits() const {
    return units;
}


