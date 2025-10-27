#include "Unit.h"
#include "CellGrid.h"
#include "Pathfinding.h"
#include <random>
#include <iostream>
#include <SDL.h>
#include <limits>
#include "Buildings.h"

HouseManager* HouseManager = nullptr;


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
		// Check if at house location to eat from stored food
		int gridX, gridY;
		cellGrid.pixelToGrid(x, y, gridX, gridY);
		
		if (isAtHouse(gridX, gridY) && g_HouseManager) {
			// Try to eat from house storage
			for (auto& house : g_HouseManager->houses) {
				if (house.ownerUnitId == id && !house.foodIds.empty()) {
					// Eat food from house
					int foodId = house.foodIds.back();
					house.foodIds.pop_back();
					hunger = 100;
					std::cout << "Unit " << name << " (id " << id << ") ate food " << foodId << " from house\n";
					actionQueue.pop();
					break;
				}
			}
			// If we're here and didn't eat, no food in house, pop action
			if (!actionQueue.empty() && actionQueue.top().type == ActionType::Eat) {
				actionQueue.pop();
			}
		} else if (path.empty()) {
			// Not at house and no path - need to path to house
			auto newPath = aStarFindPath(gridX, gridY, houseGridX, houseGridY, cellGrid);
			if (!newPath.empty()) {
				path = newPath;
			} else {
				// Can't reach house, give up
				actionQueue.pop();
			}
		}
		// If path is not empty, keep the Eat action and continue moving to house
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
	case ActionType::BringFoodToHouse: {
		int gridX, gridY;
		cellGrid.pixelToGrid(x, y, gridX, gridY);
		
		if (carryingFoodId == -1) {
			// Phase 1: Not carrying food, need to pick it up
			// Find food at this location
			auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
				int fx, fy;
				cellGrid.pixelToGrid(food.x, food.y, fx, fy);
				return fx == gridX && fy == gridY;
			});
			
			if (it != foods.end()) {
				// Pick up the food
				carryingFoodId = it->foodId;
				foods.erase(it);
				std::cout << "Unit " << name << " picked up food " << carryingFoodId << "\n";
				// Clear path and immediately create path to house
				path.clear();
				auto newPath = aStarFindPath(gridX, gridY, houseGridX, houseGridY, cellGrid);
				if (!newPath.empty()) {
					path = newPath;
				} else {
					// Can't reach house - drop food and give up
					std::cout << "Unit " << name << " cannot reach house, dropping food " << carryingFoodId << "\n";
					carryingFoodId = -1;
					actionQueue.pop();
				}
			} else if (path.empty()) {
				// No food here and no path - give up
				actionQueue.pop();
			}
		} else {
			// Phase 2: Carrying food, need to store it at house
			if (isAtHouse(gridX, gridY)) {
				// At house - store the food
				if (g_HouseManager) {
					for (auto& house : g_HouseManager->houses) {
						if (house.ownerUnitId == id) {
							house.foodIds.push_back(carryingFoodId);
							std::cout << "Unit " << name << " stored food " << carryingFoodId << " in house\n";
							carryingFoodId = -1;
							actionQueue.pop();
							break;
						}
					}
				}
			} else if (path.empty()) {
				// Need to path to house
				auto newPath = aStarFindPath(gridX, gridY, houseGridX, houseGridY, cellGrid);
				if (!newPath.empty()) {
					path = newPath;
				} else {
					// Can't reach house - drop food and give up
					std::cout << "Unit " << name << " cannot reach house, dropping food " << carryingFoodId << "\n";
					carryingFoodId = -1;
					actionQueue.pop();
				}
			}
		}
		break;
	}
	case ActionType::SellFoodAtMarket: {
		int gridX, gridY;
		cellGrid.pixelToGrid(x, y, gridX, gridY);
		
		if (carryingFoodId == -1) {
			// Phase 1: Get food from house to sell
			if (isAtHouse(gridX, gridY) && g_HouseManager) {
				// Try to get food from house
				for (auto& house : g_HouseManager->houses) {
					if (house.ownerUnitId == id && !house.foodIds.empty()) {
						carryingFoodId = house.foodIds.back();
						house.foodIds.pop_back();
						std::cout << "Unit " << name << " took food " << carryingFoodId << " from house to sell\n";
						path.clear();
						break;
					}
				}
				if (carryingFoodId == -1) {
					// No food in house to sell
					std::cout << "Unit " << name << " has no food to sell\n";
					actionQueue.pop();
				}
			} else if (path.empty()) {
				// Path to house to get food
				auto newPath = aStarFindPath(gridX, gridY, houseGridX, houseGridY, cellGrid);
				if (!newPath.empty()) {
					path = newPath;
				} else {
					std::cout << "Unit " << name << " cannot reach house to get food\n";
					actionQueue.pop();
				}
			}
		} else {
			// Phase 2: Go to market and sell food
			if (g_MarketManager) {
				Market* market = g_MarketManager->getMarketAt(gridX, gridY);
				if (market) {
					// At market - sell the food
					int salePrice = market->foodPrice;
					market->foodStock++;
					market->coins -= salePrice;
					
					// Add coins to house
					if (g_HouseManager) {
						for (auto& house : g_HouseManager->houses) {
							if (house.ownerUnitId == id) {
								house.coins += salePrice;
								std::cout << "Unit " << name << " sold food " << carryingFoodId 
										  << " for " << salePrice << " coins. House now has " 
										  << house.coins << " coins\n";
								break;
							}
						}
					}
					carryingFoodId = -1;
					actionQueue.pop();
				} else if (path.empty()) {
					// Find nearest market and path to it
					if (!g_MarketManager->markets.empty()) {
						int minDist = std::numeric_limits<int>::max();
						int nearestMarketX = 0, nearestMarketY = 0;
						
						for (const auto& m : g_MarketManager->markets) {
							int dist = abs(m.gridX - gridX) + abs(m.gridY - gridY);
							if (dist < minDist) {
								minDist = dist;
								nearestMarketX = m.gridX;
								nearestMarketY = m.gridY;
							}
						}
						
						auto newPath = aStarFindPath(gridX, gridY, nearestMarketX, nearestMarketY, cellGrid);
						if (!newPath.empty()) {
							path = newPath;
						} else {
							std::cout << "Unit " << name << " cannot reach market\n";
							carryingFoodId = -1;
							actionQueue.pop();
						}
					}
				}
			}
		}
		break;
	}
	case ActionType::BuyFoodAtMarket: {
		int gridX, gridY;
		cellGrid.pixelToGrid(x, y, gridX, gridY);
		
		if (g_MarketManager) {
			Market* market = g_MarketManager->getMarketAt(gridX, gridY);
			if (market) {
				// At market - try to buy food
				if (market->foodStock > 0 && g_HouseManager) {
					// Check if unit has enough coins
					for (auto& house : g_HouseManager->houses) {
						if (house.ownerUnitId == id) {
							if (house.coins >= market->foodPrice) {
								// Buy the food
								static int nextFoodId = 1000; // Use high IDs for bought food
								int boughtFoodId = nextFoodId++;
								house.coins -= market->foodPrice;
								market->foodStock--;
								market->coins += market->foodPrice;
								carryingFoodId = boughtFoodId;
								
								std::cout << "Unit " << name << " bought food " << boughtFoodId 
										  << " for " << market->foodPrice << " coins. House now has " 
										  << house.coins << " coins\n";
								
								// Now need to bring food home
								path.clear();
								actionQueue.pop();
								addAction(Action(ActionType::BringFoodToHouse, 9));
							} else {
								std::cout << "Unit " << name << " doesn't have enough coins (" 
										  << house.coins << " < " << market->foodPrice << ")\n";
								actionQueue.pop();
							}
							break;
						}
					}
				} else {
					std::cout << "Market is out of stock\n";
					actionQueue.pop();
				}
			} else if (path.empty()) {
				// Find nearest market and path to it
				if (!g_MarketManager->markets.empty()) {
					int minDist = std::numeric_limits<int>::max();
					int nearestMarketX = 0, nearestMarketY = 0;
					
					for (const auto& m : g_MarketManager->markets) {
						int dist = abs(m.gridX - gridX) + abs(m.gridY - gridY);
						if (dist < minDist) {
							minDist = dist;
							nearestMarketX = m.gridX;
							nearestMarketY = m.gridY;
						}
					}
					
					auto newPath = aStarFindPath(gridX, gridY, nearestMarketX, nearestMarketY, cellGrid);
					if (!newPath.empty()) {
						path = newPath;
					} else {
						std::cout << "Unit " << name << " cannot reach market\n";
						actionQueue.pop();
					}
				} else {
					std::cout << "No markets available\n";
					actionQueue.pop();
				}
			}
		}
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
        // Path to the food to bring it home
        auto newPath = aStarFindPath(gridX, gridY, foodGridX, foodGridY, cellGrid);
        if (!newPath.empty()) {
            path = newPath;
            // Add BringFoodToHouse action with priority 9
            addAction(Action(ActionType::BringFoodToHouse, 9));
        }
    }
}

void Unit::tryEatFromHouse() {
    // Check if house has food
    if (g_HouseManager) {
        for (auto& house : g_HouseManager->houses) {
            if (house.ownerUnitId == id && !house.foodIds.empty()) {
                // House has food, add Eat action
                addAction(Action(ActionType::Eat, 10)); // High priority
                return;
            }
        }
    }
}

bool Unit::isAtHouse(int gridX, int gridY) const {
    // Check if unit is within their house (3x3 area)
    return (gridX >= houseGridX && gridX < houseGridX + 3 &&
            gridY >= houseGridY && gridY < houseGridY + 3);
}