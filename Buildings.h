#pragma once
#include <vector>
#include <string>




struct House {
    int ownerUnitId;
    int gridX, gridY; // Top-left of 3x3 area
    std::vector<int> foodIds; // For future use

    House(int ownerId, int x, int y)
        : ownerUnitId(ownerId), gridX(x), gridY(y) {}


};

class HouseManager {
public:
    std::vector<House> houses;

    void addHouse(const House& s) { houses.push_back(s); }
    // Add more as needed
};

// Global house manager instance
extern HouseManager* g_HouseManager;
