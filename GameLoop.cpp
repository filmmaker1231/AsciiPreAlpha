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
#include <algorithm> // for find_if
#include <queue>

// Helper function to clear a seller's SellAtMarket action
static void clearSellerAction(Unit& seller) {
    seller.isSelling = false;
    seller.sellingStallX = -1;
    seller.sellingStallY = -1;
    // Clear SellAtMarket action if it's at the top
    if (!seller.actionQueue.empty()) {
        const Action& topAction = seller.actionQueue.top();
        if (topAction.type == ActionType::SellAtMarket) {
            seller.actionQueue.pop();
        }
    }
}

void runMainLoop(sdl& app) {
    bool running = true;
    SDL_Event event;
    static int frameCounter = 0;
    const int HUNGER_CHECK_FRAMES = 60; // Check hunger once every 60 frames (~1 second at 60 FPS)

    // Timing constants (ms)
    const Uint32 HUNGER_DECAY_MS = 10000;   // Decrease hunger by 1 every 10 seconds
    const Uint32 MORALITY_UPDATE_MS = 1000; // Update morality every 1 second
    const Uint32 SELLER_ABANDON_MS = 200000; // 200 seconds -> 200,000 ms
    const Uint32 FIGHT_CLAMP_MS = 2000;     // 2 seconds clamp during fight
    const Uint32 HOUSE_PRINT_MS = 10000;    // Print house contents every 10 seconds

    static Uint32 lastHousePrintTime = 0;

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

        // --- HOUSE INVENTORY PRINTING LOGIC ---
        if (now - lastHousePrintTime >= HOUSE_PRINT_MS) {
            if (g_HouseManager && app.unitManager) {
                for (const auto& house : g_HouseManager->houses) {
                    // Find the unit that owns this house
                    std::string ownerName = "Unknown";
                    for (const auto& unit : app.unitManager->getUnits()) {
                        if (unit.id == house.ownerUnitId) {
                            ownerName = unit.name;
                            break;
                        }
                    }
                    
                    // Count food items in the house
                    int foodCount = 0;
                    for (int dx = 0; dx < 3; ++dx) {
                        for (int dy = 0; dy < 3; ++dy) {
                            if (house.foodIds[dx][dy] != -1) {
                                foodCount++;
                            }
                        }
                    }
                    
                    // Count seed items in the house
                    int seedCount = house.countSeeds();
                    
                    // Count coin items in the house
                    int coinCount = house.countCoins();
                    
                    // Print the house inventory
                    std::cout << ownerName << "'s house has " << foodCount << " food, " 
                              << seedCount << " seeds, and " << coinCount << " coins in it" << std::endl;
                }
            }
            lastHousePrintTime = now;
        }

        // --- MARKET STALL ABANDONMENT LOGIC ---
        if (g_MarketManager) {
            for (auto& market : g_MarketManager->markets) {
                for (int dx = 0; dx < 3; ++dx) {
                    for (int dy = 0; dy < 3; ++dy) {
                        int &stallFoodId = market.stallFoodIds[dx][dy];
                        int &stallSellerId = market.stallSellerIds[dx][dy];
                        Uint32 &stallAbandonTime = market.stallAbandonTimes[dx][dy];

                        if (stallFoodId != -1) {
                            // Check if seller is present at the stall
                            bool sellerPresent = false;
                            if (stallSellerId != -1 && app.unitManager) {
                                for (const auto& unit : app.unitManager->getUnits()) {
                                    if (unit.id == stallSellerId &&
                                        unit.isSelling &&
                                        unit.sellingStallX == market.gridX + dx &&
                                        unit.sellingStallY == market.gridY + dy) {
                                        sellerPresent = true;
                                        // Reset abandon timer if seller is present
                                        if (stallAbandonTime != 0) stallAbandonTime = 0;
                                        break;
                                    }
                                }
                            }

                            if (!sellerPresent) {
                                // Seller not present -> start or continue abandon timer
                                if (stallAbandonTime == 0) {
                                    stallAbandonTime = now;
                                } else if (now - stallAbandonTime >= SELLER_ABANDON_MS) {
                                    // 200 seconds have passed -> make food free
                                    int foodId = stallFoodId;
                                    if (app.foodManager) {
                                        auto& foods = app.foodManager->getFood();
                                        auto foodIt = std::find_if(foods.begin(), foods.end(),
                                            [&](const Food& food) { return food.foodId == foodId; });
                                        if (foodIt != foods.end()) {
                                            foodIt->ownedByHouseId = -1;
                                            foodIt->carriedByUnitId = -1;
                                            std::cout << "Food (id " << foodId << ") at market stall has been abandoned and is now free.\n";
                                        }
                                    }

                                    // Clear seller's selling status if seller unit still exists
                                    int sellerIdToCheck = stallSellerId;
                                    
                                    // Clear the stall and reset fields
                                    stallFoodId = -1;
                                    stallSellerId = -1;
                                    // reset timer (only once)
                                    stallAbandonTime = 0;

                                    if (sellerIdToCheck != -1 && app.unitManager) {
                                        for (auto& unit : app.unitManager->getUnits()) {
                                            if (unit.id == sellerIdToCheck && unit.isSelling) {
                                                clearSellerAction(unit);
                                            }
                                        }
                                    }
                                }
                            }
                        } // end if stallFoodId != -1
                    }
                }
            }
        }

        // Process units
        if (app.unitManager) {
            auto& unitsRef = app.unitManager->getUnits();
            for (auto& unit : unitsRef) {

                // --- CHECK FOR COINS TO BRING HOME AFTER SELLING ---
                if (!unit.receivedCoins.empty() && unit.carriedCoinId == -1) {
                    bool alreadyBringingCoin = false;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::BringCoinToHouse) alreadyBringingCoin = true;
                    }
                    if (!alreadyBringingCoin) {
                        unit.addAction(Action(ActionType::BringCoinToHouse, 3));
                    }
                }

                // --- AUTO BRING FOOD TO HOUSE LOGIC ---
                bool alreadyBringingFood = false;
                if (!unit.actionQueue.empty()) {
                    const Action& current = unit.actionQueue.top();
                    if (current.type == ActionType::BringItemToHouse && current.itemType == "food") {
                        alreadyBringingFood = true;
                    }
                }

                if (!alreadyBringingFood && g_HouseManager && app.foodManager && !app.foodManager->getFood().empty()) {
                    for (auto& house : g_HouseManager->houses) {
                        if (house.ownerUnitId == unit.id &&
                            house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
                            if (house.hasSpace()) {
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
                if (now - unit.lastHungerUpdate >= HUNGER_DECAY_MS) {
                    if (unit.hunger > 0) {
                        unit.hunger -= 1;
                    }
                    unit.lastHungerUpdate = now;
                }
                // --- HUNGER LOGIC END ---

                // --- MORALITY LOGIC START ---
                if (now - unit.lastMoralityUpdate >= MORALITY_UPDATE_MS) {
                    if (unit.hunger < 50) {
                        if (unit.morality > 0) unit.morality -= 1;
                    } else if (unit.hunger > 50) {
                        if (unit.morality < 100) unit.morality += 1;
                    }
                    unit.lastMoralityUpdate = now;
                }
                // --- MORALITY LOGIC END ---

                // Print debug every 30 seconds
                if (now - unit.lastHungerDebugPrint >= 30000) {
                    std::cout << "Unit " << unit.name
                              << " (id " << unit.id << ") hunger: " << unit.hunger
                              << " Morality:" << unit.morality << " Health: " << unit.health << std::endl;
                    unit.lastHungerDebugPrint = now;
                }

                // --- EAT FROM HOUSE LOGIC ---
                bool tryingToEatFromHouse = false;
                if ((frameCounter % HUNGER_CHECK_FRAMES == 0) && unit.hunger < 50) {
                    bool alreadyEatingFromHouse = false;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::EatFromHouse) alreadyEatingFromHouse = true;
                    }
                    if (!alreadyEatingFromHouse && g_HouseManager) {
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
                bool tryingToSteal = false;
                if ((frameCounter % HUNGER_CHECK_FRAMES == 0) && unit.morality < 10 && unit.hunger <= 30) {
                    bool alreadyStealing = false;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::StealFood) alreadyStealing = true;
                    }
                    if (!alreadyStealing && g_HouseManager) {
                        for (auto& house : g_HouseManager->houses) {
                            if (house.hasFood()) {
                                unit.addAction(Action(ActionType::StealFood, 8));
                                tryingToSteal = true;
                                break;
                            }
                        }
                    }
                }

                // --- FIND FOOD IN WORLD LOGIC ---
                if ((frameCounter % HUNGER_CHECK_FRAMES == 0) && unit.hunger <= 99 && !tryingToEatFromHouse && !tryingToSteal) {
                    bool alreadySeekingFood = false;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::Eat) alreadySeekingFood = true;
                    }
                    if (!alreadySeekingFood) {
                        unit.tryFindAndPathToFood(*app.cellGrid, app.foodManager->getFood());
                    }
                }

                // --- AUTO COLLECT SEEDS LOGIC (Priority 3) ---
                if (app.seedManager && !app.seedManager->getSeeds().empty()) {
                    bool alreadyCollectingSeed = false;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::CollectSeed) alreadyCollectingSeed = true;
                    }
                    if (!alreadyCollectingSeed && g_HouseManager) {
                        for (auto& house : g_HouseManager->houses) {
                            if (house.ownerUnitId == unit.id &&
                                house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
                                if (house.hasSpace()) {
                                    bool hasCollectableSeed = false;
                                    for (const auto& seed : app.seedManager->getSeeds()) {
                                        if (seed.carriedByUnitId == -1 &&
                                            (seed.ownedByHouseId == -1 || seed.ownedByHouseId == unit.id)) {
                                            bool isPlantedInFarm = false;
                                            if (g_FarmManager) {
                                                for (const auto& farm : g_FarmManager->farms) {
                                                    for (int dx = 0; dx < 3 && !isPlantedInFarm; ++dx) {
                                                        for (int dy = 0; dy < 3; ++dy) {
                                                            if (farm.plantIds[dx][dy] == seed.seedId) {
                                                                isPlantedInFarm = true;
                                                                break;
                                                            }
                                                        }
                                                    }
                                                    if (isPlantedInFarm) break;
                                                }
                                            }
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

                // --- AUTO COLLECT COINS LOGIC (Priority 3) ---
                if (app.coinManager && !app.coinManager->getCoins().empty()) {
                    bool alreadyCollectingCoin = false;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::CollectCoin) alreadyCollectingCoin = true;
                    }
                    if (!alreadyCollectingCoin && g_HouseManager) {
                        for (auto& house : g_HouseManager->houses) {
                            if (house.ownerUnitId == unit.id &&
                                house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
                                if (house.hasSpace()) {
                                    int unitGridX, unitGridY;
                                    app.cellGrid->pixelToGrid(unit.x, unit.y, unitGridX, unitGridY);
                                    bool hasCollectableCoin = false;
                                    for (const auto& coin : app.coinManager->getCoins()) {
                                        if (coin.carriedByUnitId == -1 && coin.ownedByHouseId == -1) {
                                            int coinGridX, coinGridY;
                                            app.cellGrid->pixelToGrid(coin.x, coin.y, coinGridX, coinGridY);
                                            int distance = abs(coinGridX - unitGridX) + abs(coinGridY - unitGridY);
                                            if (distance <= 20) {
                                                hasCollectableCoin = true;
                                                break;
                                            }
                                        }
                                    }
                                    if (hasCollectableCoin) {
                                        unit.addAction(Action(ActionType::CollectCoin, 3));
                                    }
                                }
                                break;
                            }
                        }
                    }
                }

                // --- AUTO BUILD FARM LOGIC (Priority 4) ---
                if (g_HouseManager && g_FarmManager) {
                    bool alreadyBuildingFarm = false;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::BuildFarm) alreadyBuildingFarm = true;
                    }
                    if (!alreadyBuildingFarm) {
                        for (auto& house : g_HouseManager->houses) {
                            if (house.ownerUnitId == unit.id &&
                                house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
                                if (house.hasSeed()) {
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
                if (g_HouseManager && g_FarmManager) {
                    bool alreadyPlanting = false;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::PlantSeed) alreadyPlanting = true;
                    }
                    if (!alreadyPlanting) {
                        for (auto& farm : g_FarmManager->farms) {
                            if (farm.ownerUnitId == unit.id && farm.hasSpace()) {
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
                if (g_FarmManager) {
                    bool alreadyHarvesting = false;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::HarvestFood) alreadyHarvesting = true;
                    }
                    if (!alreadyHarvesting) {
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

                // --- AUTO SELL AT MARKET LOGIC (Priority 2) ---
                if (g_HouseManager && g_MarketManager) {
                    bool alreadySelling = false;
                    bool alreadyBringingCoin = false;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::SellAtMarket) alreadySelling = true;
                        if (current.type == ActionType::BringCoinToHouse) alreadyBringingCoin = true;
                    }
                    bool isBusyWithCoin = alreadyBringingCoin || unit.carriedCoinId != -1 || !unit.receivedCoins.empty();

                    // If unit is already selling at a stall, they should stay there waiting for buyer
                    // Only resume if they left for higher priority task and that task is now complete
                    if (!alreadySelling && !isBusyWithCoin && unit.isSelling && unit.sellingStallX != -1) {
                        // Check if the stall still has the food (not sold yet)
                        bool stallStillActive = false;
                        for (auto& market : g_MarketManager->markets) {
                            int localX = unit.sellingStallX - market.gridX;
                            int localY = unit.sellingStallY - market.gridY;
                            if (localX >= 0 && localX < 3 && localY >= 0 && localY < 3) {
                                if (market.stallSellerIds[localX][localY] == unit.id && 
                                    market.stallFoodIds[localX][localY] != -1) {
                                    stallStillActive = true;
                                    break;
                                }
                            }
                        }
                        
                        if (stallStillActive) {
                            // Resume selling at the stall
                            unit.addAction(Action(ActionType::SellAtMarket, 2));
                        } else {
                            // Stall is no longer active (food was sold or cleared)
                            unit.isSelling = false;
                            unit.sellingStallX = -1;
                            unit.sellingStallY = -1;
                        }
                    } else if (!alreadySelling && !isBusyWithCoin && !unit.isSelling && unit.carriedFoodId == -1) {
                        // Only trigger new sell action if:
                        // - Not already selling
                        // - Not busy with coin
                        // - Not currently carrying food to sell
                        // - House is full
                        for (auto& house : g_HouseManager->houses) {
                            if (house.ownerUnitId == unit.id &&
                                house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
                                if (!house.hasSpace() && house.hasFood()) {
                                    unit.addAction(Action(ActionType::SellAtMarket, 2));
                                }
                                break;
                            }
                        }
                    }
                }

                // --- AUTO BUY AT MARKET LOGIC (Priority 2) ---
                if (g_HouseManager && g_MarketManager) {
                    bool alreadyBuying = false;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::BuyAtMarket) alreadyBuying = true;
                    }
                    if (!alreadyBuying) {
                        for (auto& house : g_HouseManager->houses) {
                            if (house.ownerUnitId == unit.id &&
                                house.gridX == unit.houseGridX && house.gridY == unit.houseGridY) {
                                bool needsFood = house.hasSpace() || !house.hasFood();
                                if (needsFood && house.hasCoin()) {
                                    bool hasActiveSeller = false;
                                    for (const auto& market : g_MarketManager->markets) {
                                        if (market.hasActiveSeller()) {
                                            hasActiveSeller = true;
                                            break;
                                        }
                                    }
                                    if (hasActiveSeller) {
                                        unit.addAction(Action(ActionType::BuyAtMarket, 2));
                                    }
                                }
                                break;
                            }
                        }
                    }
                }

                // --- FIGHT LOGIC ---
                if (unit.stolenFromByUnitId != -1) {
                    Unit* thiefUnit = nullptr;
                    if (app.unitManager) {
                        for (auto& otherUnit : app.unitManager->getUnits()) {
                            if (otherUnit.id == unit.stolenFromByUnitId) {
                                thiefUnit = &otherUnit;
                                break;
                            }
                        }
                    }

                    if (thiefUnit) {
                        int unitGridX, unitGridY, thiefGridX, thiefGridY;
                        app.cellGrid->pixelToGrid(unit.x, unit.y, unitGridX, unitGridY);
                        app.cellGrid->pixelToGrid(thiefUnit->x, thiefUnit->y, thiefGridX, thiefGridY);
                        int dist = abs(thiefGridX - unitGridX) + abs(thiefGridY - unitGridY);

                        if (dist <= 5) {
                            bool alreadyFighting = false;
                            if (!unit.actionQueue.empty()) {
                                const Action& current = unit.actionQueue.top();
                                if (current.type == ActionType::Fight) alreadyFighting = true;
                            }
                            if (!alreadyFighting) {
                                unit.addAction(Action(ActionType::Fight, 9));
                                unit.fightingTargetId = thiefUnit->id;
                            }

                            if (dist <= 1 && !unit.isClamped) {
                                unit.isClamped = true;
                                thiefUnit->isClamped = true;
                                unit.fightStartTime = now;
                                thiefUnit->fightStartTime = now;

                                thiefUnit->health -= 10;
                                std::cout << unit.name << " has hit " << thiefUnit->name
                                          << " for 10 damage for stealing from them!" << std::endl;

                                unit.stolenFromByUnitId = -1;
                                unit.fightingTargetId = -1;

                                // Clear the thief's action queue (safe swap)
                                std::priority_queue<Action, std::vector<Action>, ActionComparator> emptyQ;
                                std::swap(thiefUnit->actionQueue, emptyQ);

                                // Movement speed will be restored in unclamp logic below
                            }

                            if (!unit.isClamped && (unit.path.empty() || frameCounter % 30 == 0)) {
                                auto newPath = aStarFindPath(unitGridX, unitGridY, thiefGridX, thiefGridY, *app.cellGrid);
                                if (!newPath.empty()) {
                                    unit.path = newPath;
                                    unit.moveDelay = 30; // faster pursuit
                                }
                            }
                        } else {
                            // Too far -> give up
                            unit.stolenFromByUnitId = -1;
                            unit.fightingTargetId = -1;
                            unit.moveDelay = 50;
                            if (!unit.actionQueue.empty()) {
                                const Action& current = unit.actionQueue.top();
                                if (current.type == ActionType::Fight) {
                                    if (!unit.actionQueue.empty()) unit.actionQueue.pop();
                                }
                            }
                        }
                    } else {
                        // Thief not found
                        unit.stolenFromByUnitId = -1;
                        unit.fightingTargetId = -1;
                        unit.moveDelay = 50;
                        if (!unit.actionQueue.empty()) {
                            const Action& current = unit.actionQueue.top();
                            if (current.type == ActionType::Fight) {
                                if (!unit.actionQueue.empty()) unit.actionQueue.pop();
                            }
                        }
                    }
                }

                // Handle unclamp after fight
                if (unit.isClamped && now - unit.fightStartTime >= FIGHT_CLAMP_MS) {
                    unit.isClamped = false;
                    unit.fightStartTime = 0;
                    unit.moveDelay = 50;
                    if (!unit.actionQueue.empty()) {
                        const Action& current = unit.actionQueue.top();
                        if (current.type == ActionType::Fight) {
                            if (!unit.actionQueue.empty()) unit.actionQueue.pop();
                        }
                    }
                }

                if (unit.isClamped) {
                    unit.path.clear();
                }

                // Process actions and movement
                if (!unit.actionQueue.empty() || !unit.path.empty()) {
                    unit.processAction(*app.cellGrid, app.foodManager->getFood(), app.seedManager->getSeeds(), app.coinManager->getCoins());
                }

                // Ensure a default Wander action exists
                if (unit.actionQueue.empty()) {
                    unit.addAction(Action(ActionType::Wander, 1));
                }
            } // end for units
        } // end if unitManager

        // --- TRACK THEFT VICTIMS ---
        if (app.unitManager) {
            for (auto& thief : app.unitManager->getUnits()) {
                if (thief.justStoleFromUnitId != -1) {
                    for (auto& victim : app.unitManager->getUnits()) {
                        if (victim.id == thief.justStoleFromUnitId) {
                            victim.stolenFromByUnitId = thief.id;
                            std::cout << "Victim " << victim.name << " (id " << victim.id
                                      << ") now knows that " << thief.name << " (id " << thief.id
                                      << ") stole from them" << std::endl;
                            break;
                        }
                    }
                    thief.justStoleFromUnitId = -1;
                }
            }
        }

        // --- HANDLE COIN OWNERSHIP FROM MARKET TRANSACTIONS ---
        if (app.coinManager && app.unitManager) {
            for (auto& coin : app.coinManager->getCoins()) {
                if (coin.ownedByHouseId != -1 && coin.carriedByUnitId == -1 && coin.fromMarketSale) {
                    for (auto& seller : app.unitManager->getUnits()) {
                        if (seller.id == coin.ownedByHouseId) {
                            bool alreadyAdded = false;
                            for (int receivedCoin : seller.receivedCoins) {
                                if (receivedCoin == coin.coinId) {
                                    alreadyAdded = true;
                                    break;
                                }
                            }
                            if (!alreadyAdded) {
                                seller.receivedCoins.push_back(coin.coinId);
                                std::cout << "Market: Seller " << seller.name << " (id " << seller.id
                                          << ") received coin (id " << coin.coinId << ") from sale.\n";
                                // Clear seller selling status and action
                                clearSellerAction(seller);
                                // Mark coin as processed (no longer a fresh market coin)
                                coin.fromMarketSale = false;
                            } else {
                                // If already added previously, ensure coin flag is cleared
                                coin.fromMarketSale = false;
                            }
                            break;
                        }
                    }
                }
            }
        }

        // --- DELETE DEAD UNITS ---
        if (app.unitManager) {
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

                    int deletedId = it->id;

                    // Clear theft/fight references
                    for (auto& otherUnit : units) {
                        if (otherUnit.stolenFromByUnitId == deletedId) {
                            otherUnit.stolenFromByUnitId = -1;
                            otherUnit.fightingTargetId = -1;
                        }
                        if (otherUnit.fightingTargetId == deletedId) {
                            otherUnit.fightingTargetId = -1;
                        }
                    }

                    // Clear carried items (food)
                    if (it->carriedFoodId != -1 && app.foodManager) {
                        auto foodIt = std::find_if(app.foodManager->getFood().begin(),
                                                   app.foodManager->getFood().end(),
                                                   [&](const Food& food) { return food.foodId == it->carriedFoodId; });
                        if (foodIt != app.foodManager->getFood().end()) {
                            foodIt->carriedByUnitId = -1;
                        }
                    }

                    // Clear carried seeds
                    if (it->carriedSeedId != -1 && app.seedManager) {
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
        }

        // --- RENDERING ---
        SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
        SDL_RenderClear(app.renderer);

        renderCellGrid(app.renderer, *app.cellGrid, app.showCellGrid);

        // --- RENDER HOUSES ---
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

        // --- RENDER MARKETS ---
        if (g_MarketManager) {
            SDL_SetRenderDrawColor(app.renderer, 210, 180, 140, 255); // Light tan
            for (const auto& market : g_MarketManager->markets) {
                for (int dx = 0; dx < 3; ++dx) {
                    for (int dy = 0; dy < 3; ++dy) {
                        int px, py;
                        app.cellGrid->gridToPixel(market.gridX + dx, market.gridY + dy, px, py);
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

        // Render food, seeds, coins (in that order to maintain visibility)
        if (app.foodManager) app.foodManager->renderFood(app.renderer);
        if (app.seedManager) app.seedManager->renderSeeds(app.renderer);
        if (app.coinManager) app.coinManager->renderCoins(app.renderer);

        SDL_RenderPresent(app.renderer);
        SDL_Delay(16); // ~60 FPS
    }
}
