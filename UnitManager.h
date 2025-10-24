#pragma once
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include "Unit.h"

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
    void spawnUnit(int x, int y, const std::string& name);
    
    // Render all units
    void renderUnits(SDL_Renderer* renderer);
    
    // Get units vector (for future extensions)
    const std::vector<Unit>& getUnits() const { return units; }
};

// Initialize with sample units - call this from main
void initializeGameUnits(UnitManager* unitManager);

