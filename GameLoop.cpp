#include "GameLoop.h"
#include "CellGrid.h"
#include "UnitManager.h"
#include "InputHandler.h"
#include "PathClick.h"
#include "Actions.h"
#include "Unit.h"
#include "Food.h"
#include "Buildings.h"

#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>

void runMainLoop(sdl& app) {
    bool running = true;
    SDL_Event event;
    static int frameCounter = 0;
    const int HUNGER_CHECK_FRAMES = 60; // Check hunger once every 60 frames (~1 second at 60 FPS)

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        handleInput(app);
        pathClick(app);
        ++frameCounter;

        Uint32 now = SDL_GetTicks();

        // Process units
        for (auto& unit : app.unitManager->getUnits()) {

			// --- UPDATE CARRIED FOOD POSITION ---
			// If unit is carrying food, update the food's position to follow the unit
			if (unit.carriedFoodId != -1 && app.foodManager) {
				auto& foods = app.foodManager->getFood();
				auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
					return food.foodId == unit.carriedFoodId;
				});
				if (it != foods.end()) {
					it->x = unit.x;
					it->y = unit.y;
				}
			}

			// --- UPDATE CARRIED SEED POSITION ---
			// If unit is carrying seed, update the seed's position to follow the unit
			if (unit.carriedSeedId != -1 && app.seedManager) {
				auto& seeds = app.seedManager->getSeeds();
				auto it = std::find_if(seeds.begin(), seeds.end(), [&](const Seed& seed) {
					return seed.seedId == unit.carriedSeedId;
				});
				if (it != seeds.end()) {
					it->x = unit.x;
					it->y = unit.y;
				}
			}

			// --- AUTO BRING FOOD TO HOUSE LOGIC ---
// Only if the unit is not already bringing food, and house is not full
			bool alreadyBringingFood = false;
			if (!unit.actionQueue.empty()) {
				Action current = unit.actionQueue.top();
				if (current.type == ActionType::BringItemToHouse && current.itemType == "food") {
					alreadyBringingFood = true;
				}
			}

			// Only try to bring food if there is food available (and not carried by anyone)
			if (!alreadyBringingFood && g_HouseManager && app.foodManager && !app.foodManager->getFood().empty()) {
				for (auto& house : g_HouseManager->houses) {
					if (house.ownerUnitId == unit.id &&
						house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
						if (house.hasSpace()) {
							// Check if there's any free food in the world
							bool hasFreeFood = false;
							for (const auto& food : app.foodManager->getFood()) {
								if (food.carriedByUnitId == -1 && food.ownedByHouseId == -1) {
									hasFreeFood = true;
									break;
								}
							}
							if (hasFreeFood) {
								unit.bringItemToHouse("food");
							}
						}
						break;
					}
				}
			}





            // --- HUNGER LOGIC START ---
            // Decrease hunger by 1 every 10 seconds (10000 ms)
            if (now - unit.lastHungerUpdate >= 500) {
                if (unit.hunger > 0) {
                    unit.hunger -= 1;
                }
                unit.lastHungerUpdate = now;
            }

            // Print hunger every 30 seconds (30000 ms)
            if (now - unit.lastHungerDebugPrint >= 30000) {
                std::cout << "Unit " << unit.name
                          << " (id " << unit.id << ") hunger: "
                          << unit.hunger << std::endl;
                unit.lastHungerDebugPrint = now;
            }
            // --- HUNGER LOGIC END ---



			// --- EAT FROM HOUSE LOGIC ---
			// If hunger is below 50, try to eat from house storage first
			bool tryingToEatFromHouse = false;
			if ((frameCounter % HUNGER_CHECK_FRAMES == 0) && unit.hunger < 50) {
				bool alreadyEatingFromHouse = false;
				if (!unit.actionQueue.empty()) {
					Action current = unit.actionQueue.top();
					if (current.type == ActionType::EatFromHouse) {
						alreadyEatingFromHouse = true;
					}
				}
				if (!alreadyEatingFromHouse && g_HouseManager) {
					// Check if unit has a house with food
					for (auto& house : g_HouseManager->houses) {
						if (house.ownerUnitId == unit.id &&
							house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
							if (house.hasItem("food")) {
								unit.eatFromHouse();
								tryingToEatFromHouse = true;
							}
							break;
						}
					}
				}
			}



            // If hungry and not already seeking food, try to find food from world
            // Only check this periodically to optimize performance
            // Skip if unit is trying to eat from house (hunger < 50 and house has food)
            // Note: hunger <= 99 allows units to proactively gather food even when only slightly hungry
            if ((frameCounter % HUNGER_CHECK_FRAMES == 0) && unit.hunger <= 99 && !tryingToEatFromHouse) {
                bool alreadySeekingFood = false;
                if (!unit.actionQueue.empty()) {
                    Action current = unit.actionQueue.top();
                    if (current.type == ActionType::Eat) {
                        alreadySeekingFood = true;
                    }
                }
                if (!alreadySeekingFood) {
                    unit.tryFindAndPathToFood(*app.cellGrid, app.foodManager->getFood());
                }
            }










			// --- AUTO COLLECT SEEDS LOGIC (Priority 3) ---
			// Only if there are seeds on the map and not already collecting
			if (app.seedManager && !app.seedManager->getSeeds().empty()) {
				bool alreadyCollectingSeed = false;
				if (!unit.actionQueue.empty()) {
					Action current = unit.actionQueue.top();
					if (current.type == ActionType::CollectSeed) {
						alreadyCollectingSeed = true;
					}
				}
				
				if (!alreadyCollectingSeed && g_HouseManager) {
					// Check if unit has a house with space
					for (auto& house : g_HouseManager->houses) {
						if (house.ownerUnitId == unit.id &&
							house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
							if (house.hasSpace()) {
								// Check if there's any free seed in the world (unowned or owned by me)
								bool hasCollectableSeed = false;
								for (const auto& seed : app.seedManager->getSeeds()) {
									if (seed.carriedByUnitId == -1 && 
										(seed.ownedByHouseId == -1 || seed.ownedByHouseId == unit.id)) {
										hasCollectableSeed = true;
										break;
									}
								}
								if (hasCollectableSeed) {
									unit.addAction(Action(ActionType::CollectSeed, 3));
								}
							}
							break;
						}
					}
				}
			}

			// --- AUTO BUILD FARM LOGIC (Priority 4) ---
			// Build farm if we have at least 1 seed in house and no farm yet
			if (g_HouseManager && g_FarmManager) {
				bool alreadyBuildingFarm = false;
				if (!unit.actionQueue.empty()) {
					Action current = unit.actionQueue.top();
					if (current.type == ActionType::BuildFarm) {
						alreadyBuildingFarm = true;
					}
				}
				
				if (!alreadyBuildingFarm) {
					// Check if unit has a house with seeds
					for (auto& house : g_HouseManager->houses) {
						if (house.ownerUnitId == unit.id &&
							house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
							if (house.hasSeed()) {
								// Check if farm already exists
								bool farmExists = false;
								for (const auto& farm : g_FarmManager->farms) {
									if (farm.ownerUnitId == unit.id) {
										farmExists = true;
										break;
									}
								}
								if (!farmExists) {
									unit.addAction(Action(ActionType::BuildFarm, 4));
								}
							}
							break;
						}
					}
				}
			}

			// --- AUTO PLANT SEEDS LOGIC (Priority 4) ---
			// Plant seeds from house to farm if farm has space
			if (g_HouseManager && g_FarmManager) {
				bool alreadyPlanting = false;
				if (!unit.actionQueue.empty()) {
					Action current = unit.actionQueue.top();
					if (current.type == ActionType::PlantSeed) {
						alreadyPlanting = true;
					}
				}
				
				if (!alreadyPlanting) {
					// Check if unit has a farm with space and house with seeds
					for (auto& farm : g_FarmManager->farms) {
						if (farm.ownerUnitId == unit.id && farm.hasSpace()) {
							// Check if house has seeds
							for (auto& house : g_HouseManager->houses) {
								if (house.ownerUnitId == unit.id &&
									house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
									if (house.hasSeed()) {
										unit.addAction(Action(ActionType::PlantSeed, 4));
									}
									break;
								}
							}
							break;
						}
					}
				}
			}

			// --- AUTO HARVEST FOOD LOGIC (Priority 4) ---
			// Harvest grown food from farm
			if (g_FarmManager) {
				bool alreadyHarvesting = false;
				if (!unit.actionQueue.empty()) {
					Action current = unit.actionQueue.top();
					if (current.type == ActionType::HarvestFood) {
						alreadyHarvesting = true;
					}
				}
				
				if (!alreadyHarvesting) {
					// Check if unit has a farm with grown food
					for (auto& farm : g_FarmManager->farms) {
						if (farm.ownerUnitId == unit.id) {
							Uint32 currentTime = SDL_GetTicks();
							if (farm.getFirstGrownFoodId(currentTime) != -1) {
								unit.addAction(Action(ActionType::HarvestFood, 4));
							}
							break;
						}
					}
				}
			}



            // Process queued actions - only if there's something to process
            if (!unit.actionQueue.empty() || !unit.path.empty()) {
                unit.processAction(*app.cellGrid, app.foodManager->getFood(), app.seedManager->getSeeds());
            }

            // If no actions left, re-add Wander
            if (unit.actionQueue.empty()) {
                unit.addAction(Action(ActionType::Wander, 1));
            }
        }

        // --- RENDERING ---
        SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
        SDL_RenderClear(app.renderer);

        renderCellGrid(app.renderer, *app.cellGrid, app.showCellGrid);

		// --- RENDER HOUSES ---
		// Render house tiles (brown background)
		// Food items inside houses are rendered by the FoodManager in its own pass
		if (g_HouseManager) {
			SDL_SetRenderDrawColor(app.renderer, 139, 69, 19, 255); // Brown
			for (const auto& s : g_HouseManager->houses) {
				for (int dx = 0; dx < 3; ++dx) {
					for (int dy = 0; dy < 3; ++dy) {
						int px, py;
						app.cellGrid->gridToPixel(s.gridX + dx, s.gridY + dy, px, py);
						SDL_Rect rect = { px, py, GRID_SIZE, GRID_SIZE };
						SDL_RenderFillRect(app.renderer, &rect);
					}
				}
			}
		}

		// --- RENDER FARMS ---
		// Render farm tiles (brownish-green background)
		if (g_FarmManager) {
			SDL_SetRenderDrawColor(app.renderer, 107, 142, 35, 255); // Olive drab (brownish-green)
			for (const auto& farm : g_FarmManager->farms) {
				for (int dx = 0; dx < 3; ++dx) {
					for (int dy = 0; dy < 3; ++dy) {
						int px, py;
						app.cellGrid->gridToPixel(farm.gridX + dx, farm.gridY + dy, px, py);
						SDL_Rect rect = { px, py, GRID_SIZE, GRID_SIZE };
						SDL_RenderFillRect(app.renderer, &rect);
					}
				}
			}
		}

        // Render units and their paths
        if (app.unitManager) {
            app.unitManager->renderUnits(app.renderer);
            app.unitManager->renderUnitPaths(app.renderer, *app.cellGrid);
        }

        // Render food (world food items with 'f' symbols)
        // This is rendered AFTER houses and units to ensure food is always visible on top
        if (app.foodManager) {
            app.foodManager->renderFood(app.renderer);
        }

        // Render seeds (world seed items with '.' symbols)
        // This is rendered AFTER food to ensure seeds are visible
        if (app.seedManager) {
            app.seedManager->renderSeeds(app.renderer);
        }

        SDL_RenderPresent(app.renderer);
        SDL_Delay(16); // ~60 FPS
    }
}
