#pragma once
#include <string>
#include <vector>
#include <queue>
#include "Actions.h"

class CellGrid; // Forward declaration

class Food {
public:
    int x, y;
    char symbol;
    std::string type;
    int foodValue;
    int foodId;

    Food(int x, int y, char symbol, const std::string& type, int foodValue = 50, int foodId = 0)
        : x(x), y(y), symbol(symbol), type(type), foodValue(foodValue), foodId(foodId) {}
};
