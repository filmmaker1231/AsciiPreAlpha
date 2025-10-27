#pragma once
#include <vector>
#include <string>




struct House {
    int ownerUnitId;
    int gridX, gridY; // Top-left of 3x3 area
    std::vector<int> foodIds; // Food stored in house
    int coins; // Coins stored in house

    House(int ownerId, int x, int y)
        : ownerUnitId(ownerId), gridX(x), gridY(y), coins(10) {} // Start with 10 coins


};

struct Market {
    int gridX, gridY; // Top-left of 3x3 area
    int foodStock; // Food available for purchase
    int coins; // Coins in market
    int foodPrice; // Price per food unit
    
    Market(int x, int y, int stock = 5, int startCoins = 50, int price = 3)
        : gridX(x), gridY(y), foodStock(stock), coins(startCoins), foodPrice(price) {}
};

class HouseManager {
public:
    std::vector<House> houses;

    void addHouse(const House& s) { houses.push_back(s); }
    // Add more as needed
};

class MarketManager {
public:
    std::vector<Market> markets;
    
    void addMarket(const Market& m) { markets.push_back(m); }
    
    Market* getMarketAt(int gridX, int gridY) {
        for (auto& market : markets) {
            if (gridX >= market.gridX && gridX < market.gridX + 3 &&
                gridY >= market.gridY && gridY < market.gridY + 3) {
                return &market;
            }
        }
        return nullptr;
    }
};

// Global managers
extern HouseManager* g_HouseManager;
extern MarketManager* g_MarketManager;

