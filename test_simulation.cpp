#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>

// Minimal mock structures to simulate without SDL2
struct MockAction {
    enum Type { Wander, Eat, BuildHouse, BringFoodToHouse } type;
    int priority;
    MockAction(Type t, int p) : type(t), priority(p) {}
    
    static const char* typeToString(Type t) {
        switch(t) {
            case Wander: return "Wander";
            case Eat: return "Eat";
            case BuildHouse: return "BuildHouse";
            case BringFoodToHouse: return "BringFoodToHouse";
            default: return "Unknown";
        }
    }
};

struct MockActionComparator {
    bool operator()(const MockAction& a, const MockAction& b) const {
        return a.priority < b.priority;
    }
};

struct MockFood {
    int x, y;
    int foodId;
    MockFood(int x, int y, int id) : x(x), y(y), foodId(id) {}
};

struct MockHouse {
    int ownerUnitId;
    int gridX, gridY;
    std::vector<int> foodIds;
    MockHouse(int owner, int x, int y) : ownerUnitId(owner), gridX(x), gridY(y) {}
};

struct MockUnit {
    int id;
    std::string name;
    int x, y;
    int houseGridX, houseGridY;
    int carryingFoodId;
    int hunger;
    std::vector<std::pair<int, int>> path;
    std::priority_queue<MockAction, std::vector<MockAction>, MockActionComparator> actionQueue;
    
    MockUnit(int id, const std::string& name, int x, int y, int hx, int hy) 
        : id(id), name(name), x(x), y(y), houseGridX(hx), houseGridY(hy), 
          carryingFoodId(-1), hunger(100) {}
    
    bool isAtHouse(int gridX, int gridY) const {
        return (gridX >= houseGridX && gridX < houseGridX + 3 &&
                gridY >= houseGridY && gridY < houseGridY + 3);
    }
    
    void printState() const {
        std::cout << "Unit " << name << " (id=" << id << "): pos=(" << x << "," << y << ") "
                  << "hunger=" << hunger << " carrying=" << carryingFoodId 
                  << " pathLen=" << path.size();
        if (!actionQueue.empty()) {
            MockAction top = actionQueue.top();
            std::cout << " action=" << MockAction::typeToString(top.type) << "(pri=" << top.priority << ")";
        }
        std::cout << std::endl;
    }
};

class Simulation {
private:
    std::vector<MockUnit> units;
    std::vector<MockFood> foods;
    std::vector<MockHouse> houses;
    int nextFoodId;
    
public:
    Simulation() : nextFoodId(1) {}
    
    void addUnit(int id, const std::string& name, int x, int y, int hx, int hy) {
        units.emplace_back(id, name, x, y, hx, hy);
    }
    
    void addFood(int x, int y) {
        foods.emplace_back(x, y, nextFoodId++);
        std::cout << "Added food " << (nextFoodId-1) << " at (" << x << "," << y << ")\n";
    }
    
    void addHouse(int ownerId, int x, int y) {
        houses.emplace_back(ownerId, x, y);
        std::cout << "Built house for unit " << ownerId << " at (" << x << "," << y << ")\n";
    }
    
    void simulateUnitPickupFood() {
        std::cout << "\n=== Simulating Unit Picking Up Food ===\n";
        
        // Setup: 1 unit, 1 house, 1 food
        addUnit(1, "TestUnit", 10, 10, 5, 5);
        addHouse(1, 5, 5);
        addFood(15, 15);
        
        MockUnit& unit = units[0];
        
        // Simulate unit finding food and adding BringFoodToHouse action
        std::cout << "\nStep 1: Unit finds food and adds BringFoodToHouse action\n";
        unit.actionQueue.push(MockAction(MockAction::BringFoodToHouse, 9));
        unit.path.push_back({15, 15}); // Simulate path to food
        unit.printState();
        
        // Simulate unit moving to food
        std::cout << "\nStep 2: Unit moves to food location\n";
        unit.x = 15;
        unit.y = 15;
        unit.path.clear();
        unit.printState();
        
        // Simulate processing BringFoodToHouse action - Phase 1: pickup
        std::cout << "\nStep 3: Process BringFoodToHouse - Phase 1 (pickup)\n";
        MockAction currentAction = unit.actionQueue.top();
        std::cout << "Current action: " << MockAction::typeToString(currentAction.type) << "\n";
        
        if (unit.carryingFoodId == -1) {
            std::cout << "Unit not carrying food, looking for food to pick up...\n";
            // Find food at location
            auto it = std::find_if(foods.begin(), foods.end(), [&](const MockFood& f) {
                return f.x == unit.x && f.y == unit.y;
            });
            
            if (it != foods.end()) {
                unit.carryingFoodId = it->foodId;
                std::cout << "Unit picked up food " << unit.carryingFoodId << "\n";
                foods.erase(it);
                unit.path.clear(); // Clear path as mentioned in code
                std::cout << "Path cleared after pickup\n";
            } else {
                std::cout << "ERROR: No food found at location! Path empty: " << unit.path.empty() << "\n";
                if (unit.path.empty()) {
                    std::cout << "WARNING: Would pop action here - unit gives up\n";
                }
            }
        }
        unit.printState();
        
        // Simulate next frame - Phase 2: bring to house
        std::cout << "\nStep 4: Next frame - Phase 2 (bring to house)\n";
        std::cout << "Unit is carrying food " << unit.carryingFoodId << "\n";
        std::cout << "Unit at house? " << unit.isAtHouse(unit.x, unit.y) << "\n";
        std::cout << "Path empty? " << unit.path.empty() << "\n";
        
        if (unit.path.empty() && !unit.isAtHouse(unit.x, unit.y)) {
            std::cout << "BUG FOUND: Path is empty but unit is not at house!\n";
            std::cout << "Unit position: (" << unit.x << "," << unit.y << ")\n";
            std::cout << "House position: (" << unit.houseGridX << "," << unit.houseGridY << ")\n";
            std::cout << "The code should create a path to house but the action is still active\n";
            std::cout << "The processAction will keep checking this every frame but never make progress\n";
            std::cout << "\nExpected behavior: Create path to house\n";
            // Simulate creating path
            unit.path.push_back({10, 10});
            unit.path.push_back({8, 8});
            unit.path.push_back({6, 6});
            std::cout << "Path to house created with " << unit.path.size() << " steps\n";
        }
        unit.printState();
        
        // Simulate moving to house
        std::cout << "\nStep 5: Unit moves to house\n";
        unit.x = 6;
        unit.y = 6;
        unit.path.clear();
        unit.printState();
        
        // Simulate storing food
        std::cout << "\nStep 6: Store food in house\n";
        if (unit.isAtHouse(unit.x, unit.y)) {
            for (auto& house : houses) {
                if (house.ownerUnitId == unit.id) {
                    house.foodIds.push_back(unit.carryingFoodId);
                    std::cout << "Stored food " << unit.carryingFoodId << " in house\n";
                    std::cout << "House now has " << house.foodIds.size() << " food items\n";
                    unit.carryingFoodId = -1;
                    unit.actionQueue.pop();
                    break;
                }
            }
        }
        unit.printState();
        
        std::cout << "\n=== Simulation Complete ===\n";
    }
    
    void testBringFoodLogic() {
        std::cout << "\n\n=== Testing BringFoodToHouse Logic ===\n";
        
        units.clear();
        foods.clear();
        houses.clear();
        nextFoodId = 1;
        
        // Test case: Unit picks up food, then path is cleared
        // What happens on next processAction call?
        addUnit(2, "Farmer", 20, 20, 10, 10);
        addHouse(2, 10, 10);
        addFood(20, 20); // Food at same location as unit
        
        MockUnit& unit = units[0];
        unit.actionQueue.push(MockAction(MockAction::BringFoodToHouse, 9));
        
        std::cout << "\nInitial state:\n";
        unit.printState();
        
        // Process action - pickup
        std::cout << "\nProcessing: Pickup phase\n";
        if (unit.carryingFoodId == -1) {
            auto it = std::find_if(foods.begin(), foods.end(), [&](const MockFood& f) {
                return f.x == unit.x && f.y == unit.y;
            });
            if (it != foods.end()) {
                unit.carryingFoodId = it->foodId;
                foods.erase(it);
                unit.path.clear();
                std::cout << "Picked up food, cleared path\n";
            }
        }
        unit.printState();
        
        // Process action again - bring phase
        std::cout << "\nProcessing: Bring phase (next frame)\n";
        if (unit.carryingFoodId != -1) {
            if (unit.isAtHouse(unit.x, unit.y)) {
                std::cout << "Already at house\n";
            } else if (unit.path.empty()) {
                std::cout << "Need to path to house\n";
                // This is where pathfinding should happen
                std::cout << "Creating path from (" << unit.x << "," << unit.y << ") to house (" 
                         << unit.houseGridX << "," << unit.houseGridY << ")\n";
            }
        }
        unit.printState();
    }
};

int main() {
    std::cout << "=== Food Pickup/Delivery Bug Simulation ===\n";
    std::cout << "Testing the BringFoodToHouse action logic\n\n";
    
    Simulation sim;
    sim.simulateUnitPickupFood();
    sim.testBringFoodLogic();
    
    std::cout << "\n=== ANALYSIS ===\n";
    std::cout << "The bug occurs in Unit.cpp BringFoodToHouse action:\n";
    std::cout << "1. Unit picks up food successfully\n";
    std::cout << "2. path.clear() is called (line 161 in Unit.cpp)\n";
    std::cout << "3. On next frame, the code checks if carrying food (line 167)\n";
    std::cout << "4. Then checks if at house (line 168) - false\n";
    std::cout << "5. Then checks if path.empty() (line 181) - true\n";
    std::cout << "6. Code should create path to house, but might fail\n";
    std::cout << "7. If path creation fails or there's no else clause, action stays active\n";
    std::cout << "8. Unit gets stuck with action in queue but no path\n\n";
    
    std::cout << "The issue is likely:\n";
    std::cout << "- Path creation might fail silently\n";
    std::cout << "- There's no error handling if path creation fails\n";
    std::cout << "- The action should be popped if path creation fails\n";
    
    return 0;
}
