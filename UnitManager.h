#pragma once
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include "Unit.h"
#include <iostream>

// UnitManager handles spawning and rendering units
class UnitManager {
private:
    std::vector<Unit> units;
    TTF_Font* font;

public:
    UnitManager();
    ~UnitManager();

    // Initialize font for rendering
    bool initializeFont(const char* fontPath, int fontSize);

    // Spawn a unit with @ symbol at given position
    void spawnUnit(int x, int y, const std::string& name, CellGrid* cellGrid);

    // Delete unit at given pixel position (returns true if unit was deleted)
    bool deleteUnitAt(int x, int y);

    // Render all units
    void renderUnits(SDL_Renderer* renderer);

    void renderUnitPaths(SDL_Renderer* renderer, const CellGrid& cellGrid);

    std::vector<Unit>& getUnits();
    const std::vector<Unit>& getUnits() const;

};



// Initialize with sample units - call this from main
void initializeGameUnits(UnitManager* unitManager, CellGrid* cellGrid);

// Initialize default world items - call this from main after initializeGameUnits
struct sdl; // Forward declaration
void initializeWorldItems(sdl& app);


