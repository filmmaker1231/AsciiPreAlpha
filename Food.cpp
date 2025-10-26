#include "Food.h"
#include <iostream>
#include "CellGrid.h"

FoodManager::FoodManager() : font(nullptr) {
}

FoodManager::~FoodManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool FoodManager::initializeFont(const char* fontPath, int fontSize) {
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

void FoodManager::spawnFood(int x, int y, const std::string& type) {
    static int nextFoodId = 1; // Static to ensure unique IDs
    food.emplace_back(x, y, 'f', type, 100, nextFoodId++);
    std::cout << "Spawned food '" << type << "' at (" << x << ", " << y << ") with id " << (nextFoodId-1) << std::endl;
}

bool FoodManager::deleteFoodAt(int x, int y) {
    // Use a larger click area to make it easier to select food
    const int clickRadius = 20;
    
    for (auto it = food.begin(); it != food.end(); ++it) {
        // Check if click is within the food's bounding box
        if (x >= it->x - clickRadius && x <= it->x + clickRadius &&
            y >= it->y - clickRadius && y <= it->y + clickRadius) {
            std::cout << "Deleted food '" << it->type << "' (id " << it->foodId << ") at (" << it->x << ", " << it->y << ")" << std::endl;
            food.erase(it);
            return true;
        }
    }
    return false;
}

void FoodManager::renderFood(SDL_Renderer* renderer) {
    if (!font) {
        return;
    }
    
    SDL_Color color = {255, 255, 0, 255}; // Yellow color
    
    for (const auto& foodItem : food) {
        // Create surface with the food symbol
        std::string symbolStr(1, foodItem.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) {
            continue;
        }
        
        // Create texture from surface
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {foodItem.x, foodItem.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        
        SDL_FreeSurface(surface);
    }
}

std::vector<Food>& FoodManager::getFood() {
    return food;
}

const std::vector<Food>& FoodManager::getFood() const {
    return food;
}

SeedManager::SeedManager() : font(nullptr) {
}

SeedManager::~SeedManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool SeedManager::initializeFont(const char* fontPath, int fontSize) {
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

void SeedManager::spawnSeed(int x, int y, const std::string& type) {
    static int nextSeedId = 1; // Static to ensure unique IDs
    seeds.emplace_back(x, y, type, nextSeedId++);
    std::cout << "Spawned seed '" << type << "' at (" << x << ", " << y << ") with id " << (nextSeedId-1) << std::endl;
}

void SeedManager::renderSeeds(SDL_Renderer* renderer) {
    if (!font) {
        return;
    }
    
    SDL_Color color = {255, 255, 255, 255}; // White color for seeds (high visibility)
    
    for (const auto& seedItem : seeds) {
        // Create surface with the seed symbol
        std::string symbolStr(1, seedItem.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) {
            continue;
        }
        
        // Create texture from surface
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {seedItem.x, seedItem.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        
        SDL_FreeSurface(surface);
    }
}

std::vector<Seed>& SeedManager::getSeeds() {
    return seeds;
}

const std::vector<Seed>& SeedManager::getSeeds() const {
    return seeds;
}
