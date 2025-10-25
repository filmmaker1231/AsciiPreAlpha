#pragma once
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include "Unit.h"
#include <iostream>
#include "Tiles.h"

// UnitManager handles spawning and rendering food
class FoodManager {
private:
    std::vector<Food> foods;
    TTF_Font* font;

public:
    FoodManager();
    ~FoodManager();

    // Spawn a unit with @ symbol at given position
    void spawnFood(int x, int y, const std::string& type);

    // Render food
    void renderFood(SDL_Renderer* renderer);

  
    std::vector<Food>& getFood();
    const std::vector<Food>& getFood() const;

};








