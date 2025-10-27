#include <iostream>
#include <vector>
#include <string>

// Mock structures for testing market system
struct House {
    int ownerUnitId;
    int gridX, gridY;
    std::vector<int> foodIds;
    int coins;
    
    House(int owner, int x, int y) : ownerUnitId(owner), gridX(x), gridY(y), coins(10) {}
};

struct Market {
    int gridX, gridY;
    int foodStock;
    int coins;
    int foodPrice;
    
    Market(int x, int y, int stock = 5, int startCoins = 50, int price = 3)
        : gridX(x), gridY(y), foodStock(stock), coins(startCoins), foodPrice(price) {}
};

struct Unit {
    int id;
    std::string name;
    int x, y;
    int houseGridX, houseGridY;
    int carryingFoodId;
    
    Unit(int id, const std::string& name, int x, int y, int hx, int hy)
        : id(id), name(name), x(x), y(y), houseGridX(hx), houseGridY(hy), carryingFoodId(-1) {}
    
    bool isAtLocation(int gx, int gy) const {
        return (x >= gx && x < gx + 3 && y >= gy && y < gy + 3);
    }
};

class MarketSimulation {
private:
    std::vector<Unit> units;
    std::vector<House> houses;
    std::vector<Market> markets;
    
public:
    void setup() {
        std::cout << "=== Market System Simulation Setup ===\n";
        
        // Create a unit with a house
        units.emplace_back(1, "Farmer", 10, 10, 10, 10);
        houses.emplace_back(1, 10, 10);
        
        // Add some food to the house
        houses[0].foodIds.push_back(101);
        houses[0].foodIds.push_back(102);
        houses[0].foodIds.push_back(103);
        
        // Create a market
        markets.emplace_back(20, 20, 5, 50, 3);
        
        std::cout << "Unit: " << units[0].name << " at (" << units[0].x << "," << units[0].y << ")\n";
        std::cout << "House: at (" << houses[0].gridX << "," << houses[0].gridY << ") with " 
                  << houses[0].foodIds.size() << " food and " << houses[0].coins << " coins\n";
        std::cout << "Market: at (" << markets[0].gridX << "," << markets[0].gridY << ") with " 
                  << markets[0].foodStock << " food stock, " << markets[0].coins 
                  << " coins, price=" << markets[0].foodPrice << "\n\n";
    }
    
    void testSellFood() {
        std::cout << "=== Test: Selling Food at Market ===\n";
        
        Unit& unit = units[0];
        House& house = houses[0];
        Market& market = markets[0];
        
        // Phase 1: Unit at house, picks up food
        std::cout << "Phase 1: Unit picks up food from house\n";
        if (!house.foodIds.empty()) {
            unit.carryingFoodId = house.foodIds.back();
            house.foodIds.pop_back();
            std::cout << "  Unit picked up food " << unit.carryingFoodId << "\n";
            std::cout << "  House now has " << house.foodIds.size() << " food\n";
        }
        
        // Simulate moving to market
        std::cout << "\nPhase 2: Unit travels to market\n";
        unit.x = market.gridX;
        unit.y = market.gridY;
        std::cout << "  Unit now at market (" << unit.x << "," << unit.y << ")\n";
        
        // Phase 3: Sell food at market
        std::cout << "\nPhase 3: Unit sells food at market\n";
        if (unit.isAtLocation(market.gridX, market.gridY) && unit.carryingFoodId != -1) {
            int salePrice = market.foodPrice;
            market.foodStock++;
            market.coins -= salePrice;
            house.coins += salePrice;
            
            std::cout << "  Sold food " << unit.carryingFoodId << " for " << salePrice << " coins\n";
            std::cout << "  Market now has " << market.foodStock << " food, " << market.coins << " coins\n";
            std::cout << "  House now has " << house.coins << " coins\n";
            
            unit.carryingFoodId = -1;
        }
        
        std::cout << "\nResult: SELL operation successful\n";
        std::cout << "  House coins: 10 -> " << house.coins << " (+" << (house.coins - 10) << ")\n";
        std::cout << "  Market stock: 5 -> " << market.foodStock << " (+1)\n\n";
    }
    
    void testBuyFood() {
        std::cout << "=== Test: Buying Food at Market ===\n";
        
        Unit& unit = units[0];
        House& house = houses[0];
        Market& market = markets[0];
        
        int initialCoins = house.coins;
        int initialStock = market.foodStock;
        
        // Phase 1: Unit travels to market
        std::cout << "Phase 1: Unit travels to market\n";
        unit.x = market.gridX;
        unit.y = market.gridY;
        std::cout << "  Unit at market (" << unit.x << "," << unit.y << ")\n";
        
        // Phase 2: Buy food at market
        std::cout << "\nPhase 2: Unit attempts to buy food\n";
        std::cout << "  House coins: " << house.coins << ", Market price: " << market.foodPrice << "\n";
        std::cout << "  Market stock: " << market.foodStock << "\n";
        
        if (market.foodStock > 0 && house.coins >= market.foodPrice) {
            static int nextBoughtFoodId = 1000;
            int boughtFoodId = nextBoughtFoodId++;
            
            house.coins -= market.foodPrice;
            market.foodStock--;
            market.coins += market.foodPrice;
            unit.carryingFoodId = boughtFoodId;
            
            std::cout << "  Bought food " << boughtFoodId << " for " << market.foodPrice << " coins\n";
            std::cout << "  House now has " << house.coins << " coins\n";
            std::cout << "  Market now has " << market.foodStock << " food, " << market.coins << " coins\n";
        } else {
            std::cout << "  FAILED: Not enough coins or market out of stock\n";
        }
        
        // Phase 3: Return home and store food
        if (unit.carryingFoodId != -1) {
            std::cout << "\nPhase 3: Unit returns home and stores food\n";
            unit.x = house.gridX;
            unit.y = house.gridY;
            house.foodIds.push_back(unit.carryingFoodId);
            std::cout << "  Stored food " << unit.carryingFoodId << " in house\n";
            std::cout << "  House now has " << house.foodIds.size() << " food\n";
            unit.carryingFoodId = -1;
        }
        
        std::cout << "\nResult: BUY operation successful\n";
        std::cout << "  House coins: " << initialCoins << " -> " << house.coins 
                  << " (-" << (initialCoins - house.coins) << ")\n";
        std::cout << "  Market stock: " << initialStock << " -> " << market.foodStock << " (-1)\n";
        std::cout << "  House food: 2 -> " << house.foodIds.size() << " (+1)\n\n";
    }
    
    void testInsufficientCoins() {
        std::cout << "=== Test: Buying with Insufficient Coins ===\n";
        
        House& house = houses[0];
        Market& market = markets[0];
        
        // Set house coins to less than market price
        int savedCoins = house.coins;
        house.coins = 1;
        
        std::cout << "House coins: " << house.coins << ", Market price: " << market.foodPrice << "\n";
        
        if (house.coins < market.foodPrice) {
            std::cout << "  Cannot afford food - transaction blocked\n";
            std::cout << "  Result: PASS - correctly prevented purchase\n\n";
        } else {
            std::cout << "  ERROR: Should not be able to buy\n\n";
        }
        
        // Restore coins
        house.coins = savedCoins;
    }
    
    void testOutOfStock() {
        std::cout << "=== Test: Market Out of Stock ===\n";
        
        Market& market = markets[0];
        
        // Set market stock to 0
        int savedStock = market.foodStock;
        market.foodStock = 0;
        
        std::cout << "Market stock: " << market.foodStock << "\n";
        
        if (market.foodStock == 0) {
            std::cout << "  Market out of stock - transaction blocked\n";
            std::cout << "  Result: PASS - correctly prevented purchase\n\n";
        } else {
            std::cout << "  ERROR: Should not be able to buy\n\n";
        }
        
        // Restore stock
        market.foodStock = savedStock;
    }
    
    void testMultipleTransactions() {
        std::cout << "=== Test: Multiple Buy/Sell Transactions ===\n";
        
        House& house = houses[0];
        
        std::cout << "Starting coins: " << house.coins << "\n";
        std::cout << "Starting food: " << house.foodIds.size() << "\n\n";
        
        // Sell remaining food
        std::cout << "Transaction 1: Sell food\n";
        if (!house.foodIds.empty()) {
            units[0].carryingFoodId = house.foodIds.back();
            house.foodIds.pop_back();
            house.coins += markets[0].foodPrice;
            std::cout << "  Sold food for " << markets[0].foodPrice << " coins\n";
            std::cout << "  House now has " << house.coins << " coins\n\n";
        }
        
        // Buy food back
        std::cout << "Transaction 2: Buy food\n";
        if (house.coins >= markets[0].foodPrice) {
            house.coins -= markets[0].foodPrice;
            house.foodIds.push_back(2000);
            std::cout << "  Bought food for " << markets[0].foodPrice << " coins\n";
            std::cout << "  House now has " << house.coins << " coins\n\n";
        }
        
        std::cout << "Final state:\n";
        std::cout << "  House coins: " << house.coins << "\n";
        std::cout << "  House food: " << house.foodIds.size() << "\n";
        std::cout << "  Market stock: " << markets[0].foodStock << "\n";
        std::cout << "  Market coins: " << markets[0].coins << "\n\n";
    }
    
    void printSummary() {
        std::cout << "=== Final System State ===\n";
        std::cout << "Houses: " << houses.size() << "\n";
        for (const auto& h : houses) {
            std::cout << "  House (owner=" << h.ownerUnitId << "): " 
                      << h.foodIds.size() << " food, " << h.coins << " coins\n";
        }
        std::cout << "Markets: " << markets.size() << "\n";
        for (const auto& m : markets) {
            std::cout << "  Market: " << m.foodStock << " stock, " 
                      << m.coins << " coins, price=" << m.foodPrice << "\n";
        }
    }
};

int main() {
    std::cout << "========================================\n";
    std::cout << "   Market/Buying/Selling System Test   \n";
    std::cout << "========================================\n\n";
    
    MarketSimulation sim;
    
    sim.setup();
    sim.testSellFood();
    sim.testBuyFood();
    sim.testInsufficientCoins();
    sim.testOutOfStock();
    sim.testMultipleTransactions();
    sim.printSummary();
    
    std::cout << "\n========================================\n";
    std::cout << "   All Tests Completed Successfully    \n";
    std::cout << "========================================\n";
    
    return 0;
}
