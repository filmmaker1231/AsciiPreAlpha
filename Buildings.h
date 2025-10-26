#pragma once
#include <vector>
#include <string>




struct Stockpile {
    int ownerUnitId;
    int gridX, gridY; // Top-left of 3x3 area
    std::vector<int> foodIds; // For future use

    Stockpile(int ownerId, int x, int y)
        : ownerUnitId(ownerId), gridX(x), gridY(y) {}


};

class StockpileManager {
public:
    std::vector<Stockpile> stockpiles;

    void addStockpile(const Stockpile& s) { stockpiles.push_back(s); }
    // Add more as needed
};

// Global stockpile manager instance
extern StockpileManager* StockpileManager;
