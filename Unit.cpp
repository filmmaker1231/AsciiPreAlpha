#include "Unit.h"
#include "CellGrid.h"
#include "Pathfinding.h"
#include <random>
#include <iostream>
#include <SDL.h>
#include <limits>
#include "Buildings.h"


// Global seed ID counter for all seed generation
// Note: This is safe for single-threaded SDL game. Overflow is not a practical concern
// as it would require billions of seeds to be generated in a single game session.
static int g_nextSeedId = 1;


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

void Unit::processAction(CellGrid& cellGrid, std::vector<Food>& foods, std::vector<Seed>& seeds, std::vector<Coin>& coins) {

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
            
            // Update carried item positions immediately after moving
            if (carriedFoodId != -1) {
                auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
                    return food.foodId == carriedFoodId;
                });
                if (it != foods.end()) {
                    it->x = x;
                    it->y = y;
                }
            }
            
            if (carriedSeedId != -1) {
                auto it = std::find_if(seeds.begin(), seeds.end(), [&](const Seed& seed) {
                    return seed.seedId == carriedSeedId;
                });
                if (it != seeds.end()) {
                    it->x = x;
                    it->y = y;
                }
            }
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
			// Drop seeds before eating the food
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> seedDist(1, 100);
			int numSeeds = (seedDist(gen) <= 15) ? 2 : 1; // 15% chance for 2 seeds
			
			int pixelX, pixelY;
			cellGrid.gridToPixel(gridX, gridY, pixelX, pixelY);
			
			for (int i = 0; i < numSeeds; ++i) {
				// Create new seed at food location
				Seed newSeed(pixelX, pixelY, "seed", g_nextSeedId++);
				
				// Check if this location is in the unit's home
				if (g_HouseManager) {
					for (auto& house : g_HouseManager->houses) {
						if (house.ownerUnitId == id &&
							gridX >= house.gridX && gridX < house.gridX + 3 &&
							gridY >= house.gridY && gridY < house.gridY + 3) {
							// Seed dropped in home, owned by homeowner
							newSeed.ownedByHouseId = id;
							break;
						}
					}
				}
				
				seeds.push_back(newSeed);
				std::cout << "Dropped seed " << newSeed.seedId << " at (" << gridX << ", " << gridY << ")\n";
			}
			
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
                it->x = x;  // Synchronize carried item to unit position
                it->y = y;
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
                    it->ownedByHouseId = myHouse->ownerUnitId; // Mark as owned by house owner
                    // Position food in house storage (find which slot it was placed in)
                    bool positioned = false;
                    for (int dx = 0; dx < 3 && !positioned; ++dx) {
                        for (int dy = 0; dy < 3 && !positioned; ++dy) {
                            if (myHouse->foodIds[dx][dy] == carriedFoodId) {
                                cellGrid.gridToPixel(houseGridX + dx, houseGridY + dy, it->x, it->y);
                                positioned = true;
                            }
                        }
                    }
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
					// Drop seeds before eating the food
					std::random_device rd;
					std::mt19937 gen(rd());
					std::uniform_int_distribution<> seedDist(1, 100);
					int numSeeds = (seedDist(gen) <= 15) ? 2 : 1; // 15% chance for 2 seeds
					
					int pixelX, pixelY;
					cellGrid.gridToPixel(unitGridX, unitGridY, pixelX, pixelY);
					
					for (int i = 0; i < numSeeds; ++i) {
						// Create new seed at eating location (in home)
						Seed newSeed(pixelX, pixelY, "seed", g_nextSeedId++);
						
						// Seed dropped in home, owned by homeowner
						newSeed.ownedByHouseId = id;
						
						seeds.push_back(newSeed);
						std::cout << "Dropped seed " << newSeed.seedId << " in house at (" << unitGridX << ", " << unitGridY << ")\n";
					}
					
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
	case ActionType::CollectSeed: {
		// Similar to BringItemToHouse but for seeds
		// 1. If not carrying seed, path to closest seed
		if (carriedSeedId == -1) {
			// Not carrying seed - find closest unowned or my-owned seed
			int unitGridX, unitGridY;
			cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
			
			int minDist = std::numeric_limits<int>::max();
			int closestIdx = -1;
			int seedGridX = -1, seedGridY = -1;
			
			for (size_t i = 0; i < seeds.size(); ++i) {
				// Only consider seeds that are not carried, and either unowned or owned by me
				if (seeds[i].carriedByUnitId != -1) continue;
				if (seeds[i].ownedByHouseId != -1 && seeds[i].ownedByHouseId != id) continue;
				
				// Skip seeds that are planted in any farm
				bool isPlantedInFarm = false;
				if (g_FarmManager) {
					for (const auto& farm : g_FarmManager->farms) {
						for (int dx = 0; dx < 3; ++dx) {
							for (int dy = 0; dy < 3; ++dy) {
								if (farm.plantIds[dx][dy] == seeds[i].seedId) {
									isPlantedInFarm = true;
									break;
								}
							}
							if (isPlantedInFarm) break;
						}
						if (isPlantedInFarm) break;
					}
				}
				if (isPlantedInFarm) continue;
				
				int sx, sy;
				cellGrid.pixelToGrid(seeds[i].x, seeds[i].y, sx, sy);
				int dist = abs(sx - unitGridX) + abs(sy - unitGridY);
				if (dist < minDist) {
					minDist = dist;
					closestIdx = static_cast<int>(i);
					seedGridX = sx;
					seedGridY = sy;
				}
			}
			
			if (closestIdx == -1) {
				// No seed found, give up
				actionQueue.pop();
				break;
			}
			
			// If not at seed, path to it
			if (unitGridX != seedGridX || unitGridY != seedGridY) {
				if (path.empty()) {
					auto newPath = aStarFindPath(unitGridX, unitGridY, seedGridX, seedGridY, cellGrid);
					if (!newPath.empty()) path = newPath;
				}
				break;
			}
			
			// At seed, pick it up
			if (closestIdx >= 0 && closestIdx < static_cast<int>(seeds.size())) {
				carriedSeedId = seeds[closestIdx].seedId;
				seeds[closestIdx].carriedByUnitId = id;
				seeds[closestIdx].x = x;  // Synchronize carried item to unit position
				seeds[closestIdx].y = y;
				std::cout << "Unit " << name << " picked up seed (id " << seeds[closestIdx].seedId << ") to bring home.\n";
			} else {
				// Seed was taken by someone else
				actionQueue.pop();
			}
			break;
		}
		
		// 2. If carrying seed, path to house
		int unitGridX, unitGridY;
		cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
		if (unitGridX != houseGridX || unitGridY != houseGridY) {
			if (path.empty()) {
				auto newPath = aStarFindPath(unitGridX, unitGridY, houseGridX, houseGridY, cellGrid);
				if (!newPath.empty()) path = newPath;
			}
			break;
		}
		
		// 3. At house, deposit seed if house has space
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
		
		if (myHouse && myHouse->hasSpace() && carriedSeedId != -1) {
			// Find the seed object and mark it as owned by house
			auto it = std::find_if(seeds.begin(), seeds.end(), [&](const Seed& seed) {
				return seed.seedId == carriedSeedId;
			});
			if (it != seeds.end()) {
				if (myHouse->addSeed(carriedSeedId)) {
					it->carriedByUnitId = -1;
					it->ownedByHouseId = myHouse->ownerUnitId;
					// Position seed in house storage (find which slot it was placed in)
					bool positioned = false;
					for (int dx = 0; dx < 3 && !positioned; ++dx) {
						for (int dy = 0; dy < 3 && !positioned; ++dy) {
							if (myHouse->seedIds[dx][dy] == carriedSeedId) {
								cellGrid.gridToPixel(houseGridX + dx, houseGridY + dy, it->x, it->y);
								positioned = true;
							}
						}
					}
					carriedSeedId = -1;
					std::cout << "Unit " << name << " delivered seed (id " << it->seedId << ") to house storage.\n";
				}
			}
		} else {
			std::cout << "Unit " << name << " could not deliver seed: house full or missing.\n";
		}
		actionQueue.pop();
		break;
	}
	case ActionType::BuildFarm: {
		// Build farm 1 tile away from house if at least 1 seed in house
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
		
		// Check if house has at least 1 seed
		if (!myHouse || !myHouse->hasSeed()) {
			actionQueue.pop();
			break;
		}
		
		// Check if farm already exists for this unit
		bool farmExists = false;
		if (g_FarmManager) {
			for (const auto& farm : g_FarmManager->farms) {
				if (farm.ownerUnitId == id) {
					farmExists = true;
					break;
				}
			}
		}
		
		if (farmExists) {
			actionQueue.pop();
			break;
		}
		
		// Find a suitable location exactly 1 space away from house
		int farmGridX = -1, farmGridY = -1;
		bool found = false;
		
		// Try positions 1 space away from house (4 cardinal directions)
		int offsets[4][2] = {{-4, 0}, {4, 0}, {0, -4}, {0, 4}};
		
		for (int i = 0; i < 4 && !found; ++i) {
			int testX = houseGridX + offsets[i][0];
			int testY = houseGridY + offsets[i][1];
			
			// Check if 3x3 area is walkable
			bool areaWalkable = true;
			for (int dx = 0; dx < 3 && areaWalkable; ++dx) {
				for (int dy = 0; dy < 3 && areaWalkable; ++dy) {
					if (!cellGrid.isCellWalkable(testX + dx, testY + dy)) {
						areaWalkable = false;
					}
				}
			}
			
			if (areaWalkable) {
				farmGridX = testX;
				farmGridY = testY;
				found = true;
			}
		}
		
		if (!found) {
			std::cout << "Unit " << name << " could not find suitable location for farm\n";
			actionQueue.pop();
			break;
		}
		
		// Navigate to farm location
		int unitGridX, unitGridY;
		cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
		if (unitGridX != farmGridX || unitGridY != farmGridY) {
			if (path.empty()) {
				auto newPath = aStarFindPath(unitGridX, unitGridY, farmGridX, farmGridY, cellGrid);
				if (!newPath.empty()) path = newPath;
			}
			break;
		}
		
		// Build farm
		if (g_FarmManager) {
			g_FarmManager->addFarm(Farm(id, farmGridX, farmGridY));
			std::cout << "Unit " << name << " built a farm at (" << farmGridX << ", " << farmGridY << ")\n";
		}
		actionQueue.pop();
		break;
	}
	case ActionType::PlantSeed: {
		// Plant seed from house to farm
		// 1. If not carrying seed, get seed from house
		if (carriedSeedId == -1) {
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
			
			if (!myHouse || !myHouse->hasSeed()) {
				actionQueue.pop();
				break;
			}
			
			// Navigate to house
			int unitGridX, unitGridY;
			cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
			if (unitGridX != houseGridX || unitGridY != houseGridY) {
				if (path.empty()) {
					auto newPath = aStarFindPath(unitGridX, unitGridY, houseGridX, houseGridY, cellGrid);
					if (!newPath.empty()) path = newPath;
				}
				break;
			}
			
			// Pick up seed from house
			int seedId = myHouse->getFirstSeedId();
			if (seedId != -1) {
				myHouse->removeSeedById(seedId);
				carriedSeedId = seedId;
				// Mark seed as carried
				auto it = std::find_if(seeds.begin(), seeds.end(), [&](const Seed& seed) {
					return seed.seedId == seedId;
				});
				if (it != seeds.end()) {
					it->carriedByUnitId = id;
					it->x = x;  // Synchronize carried item to unit position
					it->y = y;
					std::cout << "Unit " << name << " picked up seed (id " << seedId << ") from house to plant.\n";
				}
			}
			break;
		}
		
		// 2. If carrying seed, navigate to farm and plant it
		Farm* myFarm = nullptr;
		int farmGridX = -1, farmGridY = -1;
		if (g_FarmManager) {
			for (auto& farm : g_FarmManager->farms) {
				if (farm.ownerUnitId == id) {
					myFarm = &farm;
					farmGridX = farm.gridX;
					farmGridY = farm.gridY;
					break;
				}
			}
		}
		
		if (!myFarm) {
			std::cout << "Unit " << name << " has no farm to plant in\n";
			actionQueue.pop();
			break;
		}
		
		if (!myFarm->hasSpace()) {
			std::cout << "Unit " << name << "'s farm is full\n";
			actionQueue.pop();
			break;
		}
		
		// Navigate to farm
		int unitGridX, unitGridY;
		cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
		if (unitGridX != farmGridX || unitGridY != farmGridY) {
			if (path.empty()) {
				auto newPath = aStarFindPath(unitGridX, unitGridY, farmGridX, farmGridY, cellGrid);
				if (!newPath.empty()) path = newPath;
			}
			break;
		}
		
		// Plant seed in farm
		Uint32 currentTime = SDL_GetTicks();
		if (myFarm->plantSeed(carriedSeedId, currentTime)) {
			// Find the seed object and update its position
			auto it = std::find_if(seeds.begin(), seeds.end(), [&](const Seed& seed) {
				return seed.seedId == carriedSeedId;
			});
			if (it != seeds.end()) {
				it->carriedByUnitId = -1;
				// Position seed in farm (find which slot it was placed in)
				bool positioned = false;
				for (int dx = 0; dx < 3 && !positioned; ++dx) {
					for (int dy = 0; dy < 3 && !positioned; ++dy) {
						if (myFarm->plantIds[dx][dy] == carriedSeedId) {
							cellGrid.gridToPixel(farmGridX + dx, farmGridY + dy, it->x, it->y);
							positioned = true;
						}
					}
				}
				std::cout << "Unit " << name << " planted seed (id " << carriedSeedId << ") in farm.\n";
			}
			carriedSeedId = -1;
		}
		actionQueue.pop();
		break;
	}
	case ActionType::HarvestFood: {
		// Harvest grown food from farm and bring to house
		// 1. If not carrying food, get grown food from farm
		if (carriedFoodId == -1) {
			Farm* myFarm = nullptr;
			int farmGridX = -1, farmGridY = -1;
			if (g_FarmManager) {
				for (auto& farm : g_FarmManager->farms) {
					if (farm.ownerUnitId == id) {
						myFarm = &farm;
						farmGridX = farm.gridX;
						farmGridY = farm.gridY;
						break;
					}
				}
			}
			
			if (!myFarm) {
				actionQueue.pop();
				break;
			}
			
			Uint32 currentTime = SDL_GetTicks();
			int grownFoodId = myFarm->getFirstGrownFoodId(currentTime);
			if (grownFoodId == -1) {
				actionQueue.pop();
				break;
			}
			
			// Navigate to farm
			int unitGridX, unitGridY;
			cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
			if (unitGridX != farmGridX || unitGridY != farmGridY) {
				if (path.empty()) {
					auto newPath = aStarFindPath(unitGridX, unitGridY, farmGridX, farmGridY, cellGrid);
					if (!newPath.empty()) path = newPath;
				}
				break;
			}
			
			// Pick up the grown food
			// First, convert the seed to food object
			auto seedIt = std::find_if(seeds.begin(), seeds.end(), [&](const Seed& seed) {
				return seed.seedId == grownFoodId;
			});
			if (seedIt != seeds.end()) {
				// Create food at unit location (being picked up immediately)
				static int nextFoodId = 10000; // Start from high number to avoid conflicts
				Food newFood(x, y, 'f', "farmfood", 100, nextFoodId++);
				newFood.carriedByUnitId = id;
				newFood.ownedByHouseId = id;
				foods.push_back(newFood);
				carriedFoodId = newFood.foodId;
				
				// Remove seed from world
				seeds.erase(seedIt);
				
				// Remove from farm
				myFarm->removePlantById(grownFoodId);
				
				std::cout << "Unit " << name << " harvested food (id " << newFood.foodId << ") from farm.\n";
			}
			break;
		}
		
		// 2. If carrying food, bring to house
		int unitGridX, unitGridY;
		cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
		if (unitGridX != houseGridX || unitGridY != houseGridY) {
			if (path.empty()) {
				auto newPath = aStarFindPath(unitGridX, unitGridY, houseGridX, houseGridY, cellGrid);
				if (!newPath.empty()) path = newPath;
			}
			break;
		}
		
		// At house, deposit food
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
			auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
				return food.foodId == carriedFoodId;
			});
			if (it != foods.end()) {
				if (myHouse->addFood(carriedFoodId)) {
					it->carriedByUnitId = -1;
					it->ownedByHouseId = myHouse->ownerUnitId;
					// Position food in house storage
					bool positioned = false;
					for (int dx = 0; dx < 3 && !positioned; ++dx) {
						for (int dy = 0; dy < 3 && !positioned; ++dy) {
							if (myHouse->foodIds[dx][dy] == carriedFoodId) {
								cellGrid.gridToPixel(houseGridX + dx, houseGridY + dy, it->x, it->y);
								positioned = true;
							}
						}
					}
					carriedFoodId = -1;
					std::cout << "Unit " << name << " delivered harvested food (id " << it->foodId << ") to house.\n";
				}
			}
		}
		actionQueue.pop();
		break;
	}
	case ActionType::StealFood: {
		// Steal food from the nearest house with food
		// 1. Find the nearest house with food
		int unitGridX, unitGridY;
		cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
		
		int minDist = std::numeric_limits<int>::max();
		House* targetHouse = nullptr;
		int targetHouseGridX = -1, targetHouseGridY = -1;
		
		if (g_HouseManager) {
			for (auto& house : g_HouseManager->houses) {
				if (house.hasFood()) {
					// Calculate distance to house
					int dist = abs(house.gridX - unitGridX) + abs(house.gridY - unitGridY);
					if (dist < minDist) {
						minDist = dist;
						targetHouse = &house;
						targetHouseGridX = house.gridX;
						targetHouseGridY = house.gridY;
					}
				}
			}
		}
		
		if (!targetHouse) {
			// No house with food found, give up
			actionQueue.pop();
			break;
		}
		
		// 2. Navigate to the target house if not there
		if (unitGridX != targetHouseGridX || unitGridY != targetHouseGridY) {
			if (path.empty()) {
				auto newPath = aStarFindPath(unitGridX, unitGridY, targetHouseGridX, targetHouseGridY, cellGrid);
				if (!newPath.empty()) path = newPath;
			}
			break;
		}
		
		// 3. At house, steal and eat food from storage
		if (targetHouse->hasFood()) {
			int foodId = targetHouse->getFirstFoodId();
			if (foodId != -1) {
				// Find and remove the food from world
				auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
					return food.foodId == foodId;
				});
				if (it != foods.end()) {
					// Drop seeds before eating the food
					std::random_device rd;
					std::mt19937 gen(rd());
					std::uniform_int_distribution<> seedDist(1, 100);
					int numSeeds = (seedDist(gen) <= 15) ? 2 : 1; // 15% chance for 2 seeds
					
					int pixelX, pixelY;
					cellGrid.gridToPixel(unitGridX, unitGridY, pixelX, pixelY);
					
					// Seed drops are owned by the house owner (not the thief)
					for (int i = 0; i < numSeeds; ++i) {
						// Create new seed at eating location (in the victim's home)
						Seed newSeed(pixelX, pixelY, "seed", g_nextSeedId++);
						
						// Seed dropped in home, owned by the home owner (victim)
						newSeed.ownedByHouseId = targetHouse->ownerUnitId;
						
						seeds.push_back(newSeed);
						std::cout << "Dropped seed " << newSeed.seedId << " in house at (" << unitGridX << ", " << unitGridY << ") owned by unit " << targetHouse->ownerUnitId << "\n";
					}
					
					// Eat the stolen food
					hunger = 100;
					targetHouse->removeFoodById(foodId);
					foods.erase(it);
					
					// Record who was stolen from
					justStoleFromUnitId = targetHouse->ownerUnitId;
					
					// Print the stealing message
					std::cout << "Food stolen from home (" << targetHouseGridX << ", " << targetHouseGridY << ") by " << name << "\n";
				}
			}
		}
		actionQueue.pop();
		break;
	}
	case ActionType::Fight: {
		// Fight with the unit who stole from this unit
		// This action requires access to other units, handled in GameLoop.cpp
		// Here we just maintain the path and handle clamping
		// The actual fighting logic is in GameLoop.cpp
		
		// If stolenFromByUnitId is -1, the fight is over, so remove this action
		if (stolenFromByUnitId == -1) {
			actionQueue.pop();
		}
		break;
	}
	case ActionType::SellAtMarket: {
		// Sell food at a market stall
		// 1. If not carrying food, go to house and pick up food
		if (carriedFoodId == -1) {
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
			
			if (!myHouse || !myHouse->hasFood()) {
				// No house or no food to sell
				isSelling = false;
				actionQueue.pop();
				break;
			}
			
			// Navigate to house
			int unitGridX, unitGridY;
			cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
			if (unitGridX != houseGridX || unitGridY != houseGridY) {
				if (path.empty()) {
					auto newPath = aStarFindPath(unitGridX, unitGridY, houseGridX, houseGridY, cellGrid);
					if (!newPath.empty()) path = newPath;
				}
				break;
			}
			
			// Pick up food from house
			int foodId = myHouse->getFirstFoodId();
			if (foodId != -1) {
				myHouse->removeFoodById(foodId);
				carriedFoodId = foodId;
				// Mark food as carried
				auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
					return food.foodId == foodId;
				});
				if (it != foods.end()) {
					it->carriedByUnitId = id;
					it->ownedByHouseId = -1; // No longer owned by house
					it->x = x;
					it->y = y;
					std::cout << "Unit " << name << " picked up food (id " << foodId << ") to sell at market.\n";
				}
			}
			break;
		}
		
		// 2. If carrying food but not yet at a stall, find a market and navigate to empty stall
		if (!isSelling || sellingStallX == -1 || sellingStallY == -1) {
			Market* targetMarket = nullptr;
			int stallX = -1, stallY = -1;
			
			if (g_MarketManager) {
				for (auto& market : g_MarketManager->markets) {
					if (market.findEmptyStall(stallX, stallY)) {
						targetMarket = &market;
						break;
					}
				}
			}
			
			if (!targetMarket) {
				// No empty stall available, give up
				std::cout << "Unit " << name << " couldn't find empty market stall.\n";
				isSelling = false;
				actionQueue.pop();
				break;
			}
			
			// Navigate to the stall
			int targetGridX = targetMarket->gridX + stallX;
			int targetGridY = targetMarket->gridY + stallY;
			int unitGridX, unitGridY;
			cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
			
			if (unitGridX != targetGridX || unitGridY != targetGridY) {
				if (path.empty()) {
					auto newPath = aStarFindPath(unitGridX, unitGridY, targetGridX, targetGridY, cellGrid);
					if (!newPath.empty()) path = newPath;
				}
				break;
			}
			
			// Arrived at stall, set up shop
			targetMarket->stallFoodIds[stallX][stallY] = carriedFoodId;
			targetMarket->stallSellerIds[stallX][stallY] = id;
			targetMarket->stallAbandonTimes[stallX][stallY] = 0; // Seller is present
			isSelling = true;
			sellingStallX = targetGridX;
			sellingStallY = targetGridY;
			lastAtStallTime = SDL_GetTicks();
			
			// Update food position to stall
			auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
				return food.foodId == carriedFoodId;
			});
			if (it != foods.end()) {
				int px, py;
				cellGrid.gridToPixel(targetGridX, targetGridY, px, py);
				it->x = px;
				it->y = py;
				it->carriedByUnitId = -1; // Not carried anymore, it's at the stall
			}
			carriedFoodId = -1; // Not carrying anymore
			
			std::cout << "Unit " << name << " is now selling at market stall (" << targetGridX << ", " << targetGridY << ").\n";
			break;
		}
		
		// 3. At stall, wait for buyer (this action stays active)
		// Update last at stall time
		int unitGridX, unitGridY;
		cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
		if (unitGridX == sellingStallX && unitGridY == sellingStallY) {
			lastAtStallTime = SDL_GetTicks();
		}
		// Don't pop the action, seller stays here until a higher priority action comes
		break;
	}
	case ActionType::BuyAtMarket: {
		// Buy food from a market stall
		// 1. If not carrying coin, go to house and pick up coin
		if (carriedCoinId == -1 && coinInventory.empty()) {
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
			
			if (!myHouse || !myHouse->hasCoin()) {
				// No house or no coin to buy with
				actionQueue.pop();
				break;
			}
			
			// Navigate to house
			int unitGridX, unitGridY;
			cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
			if (unitGridX != houseGridX || unitGridY != houseGridY) {
				if (path.empty()) {
					auto newPath = aStarFindPath(unitGridX, unitGridY, houseGridX, houseGridY, cellGrid);
					if (!newPath.empty()) path = newPath;
				}
				break;
			}
			
			// Pick up coin from house (add to inventory, not visual carry)
			int coinId = myHouse->getFirstCoinId();
			if (coinId != -1) {
				myHouse->removeCoinById(coinId);
				coinInventory.push_back(coinId);
				std::cout << "Unit " << name << " picked up coin (id " << coinId << ") to buy at market.\n";
			}
			break;
		}
		
		// 2. If carrying coin, find a stall with a seller and navigate there
		if (carriedFoodId == -1) {
			Market* targetMarket = nullptr;
			int stallX = -1, stallY = -1;
			
			if (g_MarketManager) {
				for (auto& market : g_MarketManager->markets) {
					if (market.findActiveSellerStall(stallX, stallY)) {
						targetMarket = &market;
						break;
					}
				}
			}
			
			if (!targetMarket) {
				// No active seller, give up
				std::cout << "Unit " << name << " couldn't find active seller at market.\n";
				actionQueue.pop();
				break;
			}
			
			// Navigate to the stall
			int targetGridX = targetMarket->gridX + stallX;
			int targetGridY = targetMarket->gridY + stallY;
			int unitGridX, unitGridY;
			cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
			
			if (unitGridX != targetGridX || unitGridY != targetGridY) {
				if (path.empty()) {
					auto newPath = aStarFindPath(unitGridX, unitGridY, targetGridX, targetGridY, cellGrid);
					if (!newPath.empty()) path = newPath;
				}
				break;
			}
			
			// Arrived at stall, make the transaction
			int foodId = targetMarket->stallFoodIds[stallX][stallY];
			int sellerId = targetMarket->stallSellerIds[stallX][stallY];
			
			if (foodId == -1 || sellerId == -1 || coinInventory.empty()) {
				// Something went wrong, give up
				actionQueue.pop();
				break;
			}
			
			// Transfer coin to seller (GameLoop processes coins with ownedByHouseId set)
			int coinId = coinInventory.back();
			coinInventory.pop_back();
			
			// Mark the coin with seller's ID so GameLoop can add it to seller's receivedCoins
			// The coin's ownedByHouseId field is set below to trigger this mechanism
			
			// Pick up the food
			auto foodIt = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
				return food.foodId == foodId;
			});
			if (foodIt != foods.end()) {
				carriedFoodId = foodId;
				foodIt->carriedByUnitId = id;
				foodIt->x = x;
				foodIt->y = y;
			}
			
			// Clear the stall
			targetMarket->stallFoodIds[stallX][stallY] = -1;
			// Keep seller ID for GameLoop to find the seller
			int sellerIdTemp = targetMarket->stallSellerIds[stallX][stallY];
			targetMarket->stallSellerIds[stallX][stallY] = -1;
			targetMarket->stallAbandonTimes[stallX][stallY] = 0;
			
			// Place coin at stall for GameLoop to handle
			auto coinIt = std::find_if(coins.begin(), coins.end(), [&](const Coin& coin) {
				return coin.coinId == coinId;
			});
			if (coinIt != coins.end()) {
				int px, py;
				cellGrid.gridToPixel(targetGridX, targetGridY, px, py);
				coinIt->x = px;
				coinIt->y = py;
				coinIt->carriedByUnitId = -1;
				coinIt->ownedByHouseId = sellerIdTemp; // Mark as owned by seller
			}
			
			std::cout << "Unit " << name << " (buyer) and seller (id " << sellerIdTemp << ") have made a deal.\n";
			
			// Buyer action depends on hunger
			// This will be handled next iteration
			break;
		}
		
		// 3. If carrying food from purchase, consume or bring home
		if (carriedFoodId != -1) {
			// Check hunger level
			if (hunger < 50) {
				// Consume on the spot
				int unitGridX, unitGridY;
				cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
				
				// Drop seeds before eating
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_int_distribution<> seedDist(1, 100);
				int numSeeds = (seedDist(gen) <= 15) ? 2 : 1;
				
				int pixelX, pixelY;
				cellGrid.gridToPixel(unitGridX, unitGridY, pixelX, pixelY);
				
				for (int i = 0; i < numSeeds; ++i) {
					Seed newSeed(pixelX, pixelY, "seed", g_nextSeedId++);
					seeds.push_back(newSeed);
				}
				
				// Eat the food
				hunger = 100;
				auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
					return food.foodId == carriedFoodId;
				});
				if (it != foods.end()) {
					foods.erase(it);
				}
				carriedFoodId = -1;
				std::cout << "Unit " << name << " consumed purchased food on the spot.\n";
				actionQueue.pop();
			} else {
				// Bring food to house
				int unitGridX, unitGridY;
				cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
				if (unitGridX != houseGridX || unitGridY != houseGridY) {
					if (path.empty()) {
						auto newPath = aStarFindPath(unitGridX, unitGridY, houseGridX, houseGridY, cellGrid);
						if (!newPath.empty()) path = newPath;
					}
					break;
				}
				
				// At house, store the food
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
				
				if (myHouse && myHouse->hasSpace()) {
					auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
						return food.foodId == carriedFoodId;
					});
					if (it != foods.end()) {
						if (myHouse->addFood(carriedFoodId)) {
							it->carriedByUnitId = -1;
							it->ownedByHouseId = id;
							// Position food in house
							for (int dx = 0; dx < 3; ++dx) {
								for (int dy = 0; dy < 3; ++dy) {
									if (myHouse->foodIds[dx][dy] == carriedFoodId) {
										cellGrid.gridToPixel(houseGridX + dx, houseGridY + dy, it->x, it->y);
										break;
									}
								}
							}
							carriedFoodId = -1;
							std::cout << "Unit " << name << " stored purchased food at home.\n";
						}
					}
				}
				actionQueue.pop();
			}
		}
		break;
	}
	case ActionType::BringCoinToHouse: {
		// Bring coin from stall to house after selling
		// 1. If not carrying coin, navigate to coin location and pick it up
		if (carriedCoinId == -1 && !receivedCoins.empty()) {
			// Find a coin that's owned by this seller
			int coinId = receivedCoins.front();
			auto coinIt = std::find_if(coins.begin(), coins.end(), [&](const Coin& coin) {
				return coin.coinId == coinId && coin.ownedByHouseId == id;
			});
			
			if (coinIt == coins.end()) {
				// Coin not found or already picked up
				receivedCoins.erase(receivedCoins.begin());
				if (receivedCoins.empty()) {
					actionQueue.pop();
				}
				break;
			}
			
			// Navigate to coin
			int coinGridX, coinGridY;
			cellGrid.pixelToGrid(coinIt->x, coinIt->y, coinGridX, coinGridY);
			int unitGridX, unitGridY;
			cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
			
			if (unitGridX != coinGridX || unitGridY != coinGridY) {
				if (path.empty()) {
					auto newPath = aStarFindPath(unitGridX, unitGridY, coinGridX, coinGridY, cellGrid);
					if (!newPath.empty()) path = newPath;
				}
				break;
			}
			
			// Pick up the coin
			carriedCoinId = coinId;
			coinIt->carriedByUnitId = id;
			coinIt->x = x;
			coinIt->y = y;
			receivedCoins.erase(receivedCoins.begin());
			std::cout << "Unit " << name << " picked up coin (id " << coinId << ") from sale to bring home.\n";
			break;
		}
		
		// 2. If carrying coin, navigate to house
		if (carriedCoinId != -1) {
			int unitGridX, unitGridY;
			cellGrid.pixelToGrid(x, y, unitGridX, unitGridY);
			if (unitGridX != houseGridX || unitGridY != houseGridY) {
				if (path.empty()) {
					auto newPath = aStarFindPath(unitGridX, unitGridY, houseGridX, houseGridY, cellGrid);
					if (!newPath.empty()) path = newPath;
				}
				break;
			}
			
			// At house, store the coin
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
			
			if (myHouse && myHouse->hasSpace()) {
				auto coinIt = std::find_if(coins.begin(), coins.end(), [&](const Coin& coin) {
					return coin.coinId == carriedCoinId;
				});
				if (coinIt != coins.end()) {
					if (myHouse->addCoin(carriedCoinId)) {
						coinIt->carriedByUnitId = -1;
						coinIt->ownedByHouseId = id;
						// Position coin in house
						for (int dx = 0; dx < 3; ++dx) {
							for (int dy = 0; dy < 3; ++dy) {
								if (myHouse->coinIds[dx][dy] == carriedCoinId) {
									cellGrid.gridToPixel(houseGridX + dx, houseGridY + dy, coinIt->x, coinIt->y);
									break;
								}
							}
						}
						carriedCoinId = -1;
						std::cout << "Unit " << name << " stored coin at home.\n";
					}
				}
			}
			
			// Check if there are more coins to bring
			if (receivedCoins.empty()) {
				actionQueue.pop();
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