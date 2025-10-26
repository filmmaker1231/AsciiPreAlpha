#pragma once
#include <vector>
#include <string>

// HouseItem must be defined before House
struct HouseItem {
    std::string type;
    char symbol;
    HouseItem() : type(""), symbol(' ') {}
    HouseItem(const std::string& t, char s) : type(t), symbol(s) {}
};

struct House {
    int ownerUnitId;
    int gridX, gridY; // Top-left of 3x3 area
    HouseItem items[3][3];

    House(int ownerId, int x, int y)
        : ownerUnitId(ownerId), gridX(x), gridY(y) {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                items[i][j] = HouseItem();
    }

    // Returns true if item was added, false if full
    bool addItem(const std::string& itemType, char symbol) {
        for (int dx = 0; dx < 3; ++dx) {
            for (int dy = 0; dy < 3; ++dy) {
                if (items[dx][dy].type.empty()) {
                    items[dx][dy] = HouseItem(itemType, symbol);
                    return true;
                }
            }
        }
        return false;
    }

    // Returns true if there is at least one empty slot
    bool hasSpace() const {
        for (int dx = 0; dx < 3; ++dx)
            for (int dy = 0; dy < 3; ++dy)
                if (items[dx][dy].type.empty())
                    return true;
        return false;
    }

    // Returns true if item was removed, false if not found
    bool removeItem(const std::string& itemType) {
        for (int dx = 0; dx < 3; ++dx) {
            for (int dy = 0; dy < 3; ++dy) {
                if (items[dx][dy].type == itemType) {
                    items[dx][dy] = HouseItem();
                    return true;
                }
            }
        }
        return false;
    }

    // Returns true if house has at least one item of given type
    bool hasItem(const std::string& itemType) const {
        for (int dx = 0; dx < 3; ++dx)
            for (int dy = 0; dy < 3; ++dy)
                if (items[dx][dy].type == itemType)
                    return true;
        return false;
    }
};

class HouseManager {
public:
    std::vector<House> houses;

    void addHouse(const House& s) { houses.push_back(s); }
    // Add more as needed
};

// Global house manager instance
extern HouseManager* g_HouseManager;
