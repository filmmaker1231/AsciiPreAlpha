#include "Unit.h"
#include "CellGrid.h"
#include "Pathfinding.h"
#include <random>
#include <iostream>
#include <SDL.h>
#include <limits>
#include "Buildings.h"

HouseManager* HouseManager = nullptr;


static int findClosestFoodIndex(const Unit& unit, const std::vector<Food>& foods, const CellGrid& cellGrid, int& outFoodGridX, int& outFoodGridY) {
    int unitGridX, unitGridY;
    cellGrid.pixelToGrid(unit.x, unit.y, unitGridX, unitGridY);
    int minDist = std::numeric_limits<int>::max();
    int closestIdx = -1;
    for (size_t i = 0; i < foods.size(); ++i) {
        // Only consider food that is not carried and not owned by any house
        if (foods[i].carriedByUnitId != -1 || foods[i].ownedByHouseId != -1) {
            continue;
        }
        int fx, fy;
        cellGrid.pixelToGrid(foods[i].x, foods[i].y, fx, fy);
        int dist = abs(fx - unitGridX) + abs(fy - unitGridY);
        if (dist < minDist) {
            minDist = dist;
            closestIdx = static_cast<int>(i);
            outFoodGridX = fx;
            outFoodGridY = fy;
        }
    }
    return closestIdx;
}


void Unit::addAction(const Action& action) {
    if (actionQueue.empty()) {
        actionQueue.push(action);
        return;
    }
    int currentPriority = actionQueue.top().priority;
    if (action.priority > currentPriority) {
        // Cancel all current actions and do this one
        std::priority_queue<Action, std::vector<Action>, ActionComparator> empty;
        std::swap(actionQueue, empty);
        actionQueue.push(action);
    } else if (action.priority == currentPriority) {
        actionQueue.push(action);
    } else {
        // Lower priority, add to queue
        actionQueue.push(action);
    }
}

void Unit::processAction(CellGrid& cellGrid, std::vector<Food>& foods) {

    // First, handle any path movement (works with or without actions)
    // This allows manually-assigned paths (e.g., via P+click) to be followed
    if (!path.empty()) {
        unsigned int currentTime = SDL_GetTicks();
        // Only move if enough time has passed since last move, or if this is the first move
        if (lastMoveTime == 0 || currentTime - lastMoveTime >= moveDelay) {
            auto [nextGridX, nextGridY] = path.front();
            int nextPixelX, nextPixelY;
            cellGrid.gridToPixel(nextGridX, nextGridY, nextPixelX, nextPixelY);
            x = nextPixelX;
            y = nextPixelY;
            path.erase(path.begin());
            lastMoveTime = currentTime;
        }
    }

    // Then process any queued actions
    if (actionQueue.empty()) return;
    Action current = actionQueue.top();

    switch (current.type) {
    case ActionType::Wander: {
        // If no path, pick a random walkable cell nearby and path to it
        if (path.empty()) {
            int gridX, gridY;
            cellGrid.pixelToGrid(x, y, gridX, gridY);

            // Try up to 10 times to find a random walkable cell nearby
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(-5, 5);

            for (int attempt = 0; attempt < 10; ++attempt) {
                int dx = dist(gen);
                int dy = dist(gen);
                int nx = gridX + dx;
                int ny = gridY + dy;
                if ((dx != 0 || dy != 0) && cellGrid.isCellWalkable(nx, ny)) {
                    auto newPath = aStarFindPath(gridX, gridY, nx, ny, cellGrid);
                    if (!newPath.empty()) {
                        path = newPath;
                        break;
                    }
                }
            }
            
            // If still no path found, pop and let GameLoop re-add Wander
            if (path.empty() && !actionQueue.empty()) {
                actionQueue.pop();
            }
        }
        // If path exists, keep following it with the Wander action active
        break;
    }
	case ActionType::Eat: {
		// Check if at food location
		int gridX, gridY;
		cellGrid.pixelToGrid(x, y, gridX, gridY);

		// Find food at this location (only free food, not carried or owned)
		auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
			int fx, fy;
			cellGrid.pixelToGrid(food.x, food.y, fx, fy);
			return fx == gridX && fy == gridY && food.carriedByUnitId == -1 && food.ownedByHouseId == -1;
			});

		if (it != foods.end()) {
			// Eat the food
			hunger = 100;
			foods.erase(it);
			std::cout << "Unit " << name << " (id " << id << ") ate food at (" << gridX << ", " << gridY << ")\n";
			actionQueue.pop();
		} else if (path.empty()) {
			// No food here and no path to follow - give up on this Eat action
			// (food was likely taken by another unit or unreachable)
			actionQueue.pop();
		}
		// If path is not empty, keep the Eat action and continue moving toward food
		break;
	}
	case ActionType::BuildHouse: {
		int gridX, gridY;
		cellGrid.pixelToGrid(x, y, gridX, gridY);
		if (gridX != houseGridX || gridY != houseGridY) {
			if (path.empty()) {
				auto newPath = aStarFindPath(gridX, gridY, houseGridX, houseGridY, cellGrid);
				if (!newPath.empty()) path = newPath;
			}
			// Wait for movement to finish
			break;
		}
		// Build house: add to HouseManager without marking cells as non-walkable
		if (g_HouseManager) {
			g_HouseManager->addHouse(House(id, houseGridX, houseGridY));
		}
		std::cout << "Unit " << name << " built a house at (" << houseGridX << ", " << houseGridY << ")\n";
		actionQueue.pop();
		break;
	}
	case ActionType::BringItemToHouse: {
    // Only support "food" for now, but extensible
    if (current.itemType == "food") {
        // 1. If not carrying food, path to closest food
        if (carriedFoodId == -1) {
            // Not carrying food
            int foodGridX = -1, foodGridY = -1;
            int closestIdx = findClosestFoodIndex(*this, foods, cellGrid, foodGridX, foodGridY);
            if (closestIdx == -1) {
                // No food found, give up
                actionQueue.pop();
                break;
            }
            int unitGridX, unitGridY;
            cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
            // If not at food, path to it
            if (unitGridX != foodGridX || unitGridY != foodGridY) {
                if (path.empty()) {
                    auto newPath = aStarFindPath(unitGridX, unitGridY, foodGridX, foodGridY, cellGrid);
                    if (!newPath.empty()) path = newPath;
                }
                break;
            }
            // At food, pick it up (don't delete, just mark as carried)
            auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
                int fx, fy;
                cellGrid.pixelToGrid(food.x, food.y, fx, fy);
                return fx == foodGridX && fy == foodGridY && food.carriedByUnitId == -1;
            });
            if (it != foods.end()) {
                carriedFoodId = it->foodId;
                it->carriedByUnitId = id;
                inventory.push_back("food"); // Keep for backward compatibility
                std::cout << "Unit " << name << " picked up food (id " << it->foodId << ") to bring home.\n";
            } else {
                // Food was taken by someone else
                actionQueue.pop();
            }
            break;
        }

        // 2. If carrying food, path to house
        int unitGridX, unitGridY;
        cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
        if (unitGridX != houseGridX || unitGridY != houseGridY) {
            if (path.empty()) {
                auto newPath = aStarFindPath(unitGridX, unitGridY, houseGridX, houseGridY, cellGrid);
                if (!newPath.empty()) path = newPath;
            }
            break;
        }

        // 3. At house, deposit food if house has space
        House* myHouse = nullptr;
        if (g_HouseManager) {
            for (auto& house : g_HouseManager->houses) {
                if (house.ownerUnitId == id &&
                    house.gridX == houseGridX && house.gridY == houseGridY) {
                    myHouse = &house;
                    break;
                }
            }
        }
        if (myHouse && myHouse->hasSpace() && carriedFoodId != -1) {
            // Find the food object and mark it as owned by house
            auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
                return food.foodId == carriedFoodId;
            });
            if (it != foods.end()) {
                if (myHouse->addFood(carriedFoodId)) {
                    it->carriedByUnitId = -1;
                    it->ownedByHouseId = id; // Mark as owned by this house
                    // Position food in house storage (use first empty slot position)
                    for (int dx = 0; dx < 3; ++dx) {
                        for (int dy = 0; dy < 3; ++dy) {
                            if (myHouse->foodIds[dx][dy] == carriedFoodId) {
                                cellGrid.gridToPixel(houseGridX + dx, houseGridY + dy, it->x, it->y);
                                goto food_positioned;
                            }
                        }
                    }
                    food_positioned:
                    carriedFoodId = -1;
                    auto invIt = std::find(inventory.begin(), inventory.end(), "food");
                    if (invIt != inventory.end()) {
                        inventory.erase(invIt);
                    }
                    std::cout << "Unit " << name << " delivered food (id " << it->foodId << ") to house storage.\n";
                }
            }
        } else {
            std::cout << "Unit " << name << " could not deliver food: house full or missing.\n";
        }
        actionQueue.pop();
    } else {
        // Future: handle other item types
        actionQueue.pop();
    }
    break;
}
	case ActionType::EatFromHouse: {
		// Navigate to house and eat food from storage
		int unitGridX, unitGridY;
		cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);

		// First, navigate to house if not there
		if (unitGridX != houseGridX || unitGridY != houseGridY) {
			if (path.empty()) {
				auto newPath = aStarFindPath(unitGridX, unitGridY, houseGridX, houseGridY, cellGrid);
				if (!newPath.empty()) path = newPath;
			}
			break;
		}

		// At house, try to eat food from storage
		House* myHouse = nullptr;
		if (g_HouseManager) {
			for (auto& house : g_HouseManager->houses) {
				if (house.ownerUnitId == id &&
					house.gridX == houseGridX && house.gridY == houseGridY) {
					myHouse = &house;
					break;
				}
			}
		}

		if (myHouse && myHouse->hasFood()) {
			int foodId = myHouse->getFirstFoodId();
			if (foodId != -1) {
				// Find and remove the food from world
				auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
					return food.foodId == foodId;
				});
				if (it != foods.end()) {
					hunger = 100;
					myHouse->removeFoodById(foodId);
					foods.erase(it); // Now we actually delete the food when eaten
					std::cout << "Unit " << name << " (id " << id << ") ate food (id " << foodId << ") from house storage\n";
				}
			}
		} else {
			std::cout << "Unit " << name << " tried to eat from house but no food available\n";
		}
		actionQueue.pop();
		break;
	}


	default:
		actionQueue.pop();
		break;
	}
}

void Unit::tryFindAndPathToFood(CellGrid& cellGrid, std::vector<Food>& foods) {
    if (foods.empty()) return;

    // Find nearest food using cell grid for accuracy
    int gridX, gridY;
    cellGrid.pixelToGrid(x, y, gridX, gridY);

    int minDist = std::numeric_limits<int>::max();
    int nearestFoodIdx = -1;
    int foodGridX = 0, foodGridY = 0;

    for (size_t i = 0; i < foods.size(); ++i) {
        // Only consider food that is not carried and not owned by any house
        if (foods[i].carriedByUnitId != -1 || foods[i].ownedByHouseId != -1) {
            continue;
        }
        int fx, fy;
        cellGrid.pixelToGrid(foods[i].x, foods[i].y, fx, fy);
        int dist = abs(fx - gridX) + abs(fy - gridY);
        if (dist < minDist) {
            minDist = dist;
            nearestFoodIdx = static_cast<int>(i);
            foodGridX = fx;
            foodGridY = fy;
        }
    }

    if (nearestFoodIdx != -1) {
        // Path to the food
        auto newPath = aStarFindPath(gridX, gridY, foodGridX, foodGridY, cellGrid);
        if (!newPath.empty()) {
            path = newPath;
            // Add Eat action with priority 9
            addAction(Action(ActionType::Eat, 9));
        }
    }
}