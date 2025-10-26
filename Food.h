#pragma once
#include <string>
#include <vector>
#include <queue>
#include <SDL.h>
#include <SDL_ttf.h>




class CellGrid; // Forward declaration
struct TTF_Font;

class Food {
public:
	std::string type;   // type of food
    int x, y;           // Position on the grid
    char symbol;        // Character to display (e.g., '@')
    int foodValue;
    int foodId;
    int carriedByUnitId; // -1 if not carried, otherwise the unit ID carrying it
    int ownedByHouseId;  // -1 if not owned, otherwise the house owner's unit ID
    
     Food(int x, int y, char symbol, const std::string& type, int foodValue = 100, int foodId = 0)
		 : x(x), y(y), symbol(symbol), type(type), foodValue(foodValue), foodId(foodId), 
		   carriedByUnitId(-1), ownedByHouseId(-1) {
	 }
};

class FoodManager {
private:
    std::vector<Food> food;
    TTF_Font* font;

public:
    FoodManager();
    ~FoodManager();

    // Initialize font for rendering
    bool initializeFont(const char* fontPath, int fontSize);

    // Spawn food with f symbol at given position
    void spawnFood(int x, int y, const std::string& type);

    // Render all units
    void renderFood(SDL_Renderer* renderer);

    
    std::vector<Food>& getFood();
    const std::vector<Food>& getFood() const;

	TTF_Font* getFont() const { return font; }

};      // Manages food

class Seed {
public:
	std::string type;   // type of seed
    int x, y;           // Position on the grid
    char symbol;        // Character to display ('.')
    int seedId;
    int carriedByUnitId; // -1 if not carried, otherwise the unit ID carrying it
    int ownedByHouseId;  // -1 if not owned, otherwise the house owner's unit ID
    
    Seed(int x, int y, const std::string& type, int seedId)
		: x(x), y(y), symbol('.'), type(type), seedId(seedId), 
		  carriedByUnitId(-1), ownedByHouseId(-1) {
	}
};

class SeedManager {
private:
    std::vector<Seed> seeds;
    TTF_Font* font;

public:
    SeedManager();
    ~SeedManager();

    // Initialize font for rendering
    bool initializeFont(const char* fontPath, int fontSize);

    // Spawn seed with . symbol at given position
    void spawnSeed(int x, int y, const std::string& type);

    // Render all seeds
    void renderSeeds(SDL_Renderer* renderer);

    
    std::vector<Seed>& getSeeds();
    const std::vector<Seed>& getSeeds() const;

};      // Manages seeds

