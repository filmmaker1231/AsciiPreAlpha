#include "GameLoop.h"
#include "CellGrid.h"
#include "UnitManager.h"
#include "InputHandler.h"
#include "PathClick.h"
#include "Actions.h"
#include "Unit.h"
#include "Food.h"
#include "Buildings.h"
#include "Pathfinding.h"

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

            // --- MORALITY LOGIC START ---
            // Update morality every 1 second (1000 ms)
            if (now - unit.lastMoralityUpdate >= 1000) {
                if (unit.hunger < 50) {
                    // Decrease morality by 1 if hunger is below 50
                    if (unit.morality > 0) {
                        unit.morality -= 1;
                    }
                } else if (unit.hunger > 50) {
                    // Increase morality by 1 if hunger is above 50
                    if (unit.morality < 100) {
                        unit.morality += 1;
                    }
                }
                unit.lastMoralityUpdate = now;
            }
            // --- MORALITY LOGIC END ---



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

			// --- STEALING LOGIC ---
			// If morality is below 10 and hunger is 30 or below, unit will steal from nearest home
			bool tryingToSteal = false;
			if ((frameCounter % HUNGER_CHECK_FRAMES == 0) && unit.morality < 10 && unit.hunger <= 30) {
				bool alreadyStealing = false;
				if (!unit.actionQueue.empty()) {
					Action current = unit.actionQueue.top();
					if (current.type == ActionType::StealFood) {
						alreadyStealing = true;
					}
				}
				if (!alreadyStealing && g_HouseManager) {
					// Check if there's any house (including others') with food
					for (auto& house : g_HouseManager->houses) {
						if (house.hasFood()) {
							// Add StealFood action with priority 8
							unit.addAction(Action(ActionType::StealFood, 8));
							tryingToSteal = true;
							break;
						}
					}
				}
			}



            // If hungry and not already seeking food, try to find food from world
            // Only check this periodically to optimize performance
            // Skip if unit is trying to eat from house (hunger < 50 and house has food)
            // Skip if unit is trying to steal (morality < 10 and hunger <= 30)
            // Note: hunger <= 99 allows units to proactively gather food even when only slightly hungry
            if ((frameCounter % HUNGER_CHECK_FRAMES == 0) && unit.hunger <= 99 && !tryingToEatFromHouse && !tryingToSteal) {
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
								// but NOT planted in any farm
								bool hasCollectableSeed = false;
								for (const auto& seed : app.seedManager->getSeeds()) {
									if (seed.carriedByUnitId == -1 && 
										(seed.ownedByHouseId == -1 || seed.ownedByHouseId == unit.id)) {
										// Check if this seed is planted in any farm
										bool isPlantedInFarm = false;
										if (g_FarmManager) {
											for (const auto& farm : g_FarmManager->farms) {
												for (int dx = 0; dx < 3; ++dx) {
													for (int dy = 0; dy < 3; ++dy) {
														if (farm.plantIds[dx][dy] == seed.seedId) {
															isPlantedInFarm = true;
															break;
														}
													}
													if (isPlantedInFarm) break;
												}
												if (isPlantedInFarm) break;
											}
										}
										// Only collect seed if it's not planted in a farm
										if (!isPlantedInFarm) {
											hasCollectableSeed = true;
											break;
										}
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



			// --- FIGHT LOGIC ---
			// If a unit had food stolen from them and the thief is within 5 tiles, fight them
			if (unit.stolenFromByUnitId != -1) {
				// Find the thief unit
				Unit* thiefUnit = nullptr;
				for (auto& otherUnit : app.unitManager->getUnits()) {
					if (otherUnit.id == unit.stolenFromByUnitId) {
						thiefUnit = &otherUnit;
						break;
					}
				}
				
				if (thiefUnit) {
					// Calculate distance to thief
					int unitGridX, unitGridY;
					app.cellGrid->pixelToGrid(unit.x, unit.y, unitGridX, unitGridY);
					int thiefGridX, thiefGridY;
					app.cellGrid->pixelToGrid(thiefUnit->x, thiefUnit->y, thiefGridX, thiefGridY);
					int dist = abs(thiefGridX - unitGridX) + abs(thiefGridY - unitGridY);
					
					// If thief is within 5 tiles, start or continue fighting
					if (dist <= 5) {
						bool alreadyFighting = false;
						if (!unit.actionQueue.empty()) {
							Action current = unit.actionQueue.top();
							if (current.type == ActionType::Fight) {
								alreadyFighting = true;
							}
						}
						
						if (!alreadyFighting) {
							// Add Fight action with priority 9
							unit.addAction(Action(ActionType::Fight, 9));
							unit.fightingTargetId = thiefUnit->id;
						}
						
						// Check if units are adjacent (distance 1 or 0)
						if (dist <= 1 && !unit.isClamped) {
							// Start the fight - clamp both units
							unit.isClamped = true;
							thiefUnit->isClamped = true;
							unit.fightStartTime = now;
							thiefUnit->fightStartTime = now;
							
							// Deal damage to the thief
							thiefUnit->health -= 10;
							std::cout << unit.name << " has hit " << thiefUnit->name 
							          << " for 10 damage for stealing from them!" << std::endl;
							
							// Clear the stolen from tracking after the hit
							unit.stolenFromByUnitId = -1;
							unit.fightingTargetId = -1;
							unit.isClamped = false;
						}
						
						// Update path to thief if not clamped
						if (!unit.isClamped && (unit.path.empty() || frameCounter % 30 == 0)) {
							// Continuously update path to follow the thief
							auto newPath = aStarFindPath(unitGridX, unitGridY, thiefGridX, thiefGridY, *app.cellGrid);
							if (!newPath.empty()) {
								unit.path = newPath;
								// Make this unit faster to catch up
								unit.moveDelay = 30; // Faster than normal (normal is 50)
							}
						}
					}
				} else {
					// Thief not found (might have been deleted), clear tracking
					unit.stolenFromByUnitId = -1;
					unit.fightingTargetId = -1;
				}
			}
			
			// Handle clamping during fight - prevent movement for 2 seconds
			if (unit.isClamped && now - unit.fightStartTime >= 2000) {
				// 2 seconds have passed, unclamp
				unit.isClamped = false;
				unit.fightStartTime = 0;
				// Restore normal speed
				unit.moveDelay = 50;
			}
			
			// Prevent movement if clamped
			if (unit.isClamped) {
				unit.path.clear();
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

		// --- TRACK THEFT VICTIMS ---
		// After all units have processed, check if any stealing occurred
		for (auto& thief : app.unitManager->getUnits()) {
			if (thief.justStoleFromUnitId != -1) {
				// Find the victim and record the theft
				for (auto& victim : app.unitManager->getUnits()) {
					if (victim.id == thief.justStoleFromUnitId) {
						victim.stolenFromByUnitId = thief.id;
						std::cout << "Victim " << victim.name << " (id " << victim.id 
						          << ") now knows that " << thief.name << " (id " << thief.id 
						          << ") stole from them" << std::endl;
						break;
					}
				}
				// Clear the flag
				thief.justStoleFromUnitId = -1;
			}
		}

		// --- DELETE DEAD UNITS ---
		// Remove units with hunger <= 0 or health <= 0
		auto& units = app.unitManager->getUnits();
		auto it = units.begin();
		while (it != units.end()) {
			bool shouldDelete = false;
			std::string deleteReason;
			
			if (it->hunger <= 0) {
				shouldDelete = true;
				deleteReason = "hunger reached 0";
			} else if (it->health <= 0) {
				shouldDelete = true;
				deleteReason = "health reached 0";
			}
			
			if (shouldDelete) {
				std::cout << "Unit " << it->name << " (id " << it->id << ") has died: " << deleteReason << std::endl;
				
				// Clean up any references to this unit
				int deletedId = it->id;
				
				// Clear any theft tracking involving this unit
				for (auto& otherUnit : units) {
					if (otherUnit.stolenFromByUnitId == deletedId) {
						otherUnit.stolenFromByUnitId = -1;
						otherUnit.fightingTargetId = -1;
					}
					if (otherUnit.fightingTargetId == deletedId) {
						otherUnit.fightingTargetId = -1;
					}
				}
				
				// Clear carried items
				if (it->carriedFoodId != -1) {
					auto foodIt = std::find_if(app.foodManager->getFood().begin(), 
					                           app.foodManager->getFood().end(),
					                           [&](const Food& food) { return food.foodId == it->carriedFoodId; });
					if (foodIt != app.foodManager->getFood().end()) {
						foodIt->carriedByUnitId = -1;
					}
				}
				
				if (it->carriedSeedId != -1) {
					auto seedIt = std::find_if(app.seedManager->getSeeds().begin(), 
					                           app.seedManager->getSeeds().end(),
					                           [&](const Seed& seed) { return seed.seedId == it->carriedSeedId; });
					if (seedIt != app.seedManager->getSeeds().end()) {
						seedIt->carriedByUnitId = -1;
					}
				}
				
				it = units.erase(it);
			} else {
				++it;
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
