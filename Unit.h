#pragma once
#include <string>

class Unit {
public:
    int x, y;           // Position on the grid
    char symbol;        // Character to display (e.g., '@')
    std::string name;   // Name of the unit

    Unit(int x, int y, char symbol, const std::string& name)
        : x(x), y(y), symbol(symbol), name(name) {}
};
