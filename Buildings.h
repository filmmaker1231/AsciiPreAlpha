#pragma once
#include <vector>
#include <string>
#include <SDL.h>




struct House {
	int ownerUnitId;
	int gridX, gridY; // Top-left of 3x3 area
	// 3x3 grid of food IDs, -1 means empty
	int foodIds[3][3];
	// 3x3 grid of seed IDs, -1 means empty
	int seedIds[3][3];
	// 3x3 grid of coin IDs, -1 means empty
	int coinIds[3][3];

	House(int ownerId, int x, int y)
		: ownerUnitId(ownerId), gridX(x), gridY(y) {
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j) {
				foodIds[i][j] = -1;
				seedIds[i][j] = -1;
				coinIds[i][j] = -1;
			}
	}

	// Returns true if food was added, false if full
	bool addFood(int foodId) {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				// Check that this slot is empty (no food, seed, OR coin)
				if (foodIds[dx][dy] == -1 && seedIds[dx][dy] == -1 && coinIds[dx][dy] == -1) {
					foodIds[dx][dy] = foodId;
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
				// Empty slot must have food, seed, AND coin all as -1
				if (foodIds[dx][dy] == -1 && seedIds[dx][dy] == -1 && coinIds[dx][dy] == -1)
					return true;
		return false;
	}

	// Returns the first food ID found, -1 if not found
	int getFirstFoodId() const {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (foodIds[dx][dy] != -1) {
					return foodIds[dx][dy];
				}
			}
		}
		return -1;
	}

	// Returns true if food was removed, false if not found
	bool removeFoodById(int foodId) {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (foodIds[dx][dy] == foodId) {
					foodIds[dx][dy] = -1;
					return true;
				}
			}
		}
		return false;
	}

	// Returns true if house has at least one food item
	bool hasFood() const {
		for (int dx = 0; dx < 3; ++dx)
			for (int dy = 0; dy < 3; ++dy)
				if (foodIds[dx][dy] != -1)
					return true;
		return false;
	}

	// Returns true if house has at least one seed item
	bool hasSeed() const {
		for (int dx = 0; dx < 3; ++dx)
			for (int dy = 0; dy < 3; ++dy)
				if (seedIds[dx][dy] != -1)
					return true;
		return false;
	}

	// Count how many seeds are in house
	int countSeeds() const {
		int count = 0;
		for (int dx = 0; dx < 3; ++dx)
			for (int dy = 0; dy < 3; ++dy)
				if (seedIds[dx][dy] != -1)
					count++;
		return count;
	}

	// Add seed to house storage (similar to addFood)
	bool addSeed(int seedId) {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				// Check that this slot is empty (no food, seed, OR coin)
				if (foodIds[dx][dy] == -1 && seedIds[dx][dy] == -1 && coinIds[dx][dy] == -1) {
					seedIds[dx][dy] = seedId;
					return true;
				}
			}
		}
		return false;
	}

	// Get first seed ID from house storage
	int getFirstSeedId() const {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (seedIds[dx][dy] != -1) {
					return seedIds[dx][dy];
				}
			}
		}
		return -1;
	}

	// Remove seed from house storage
	bool removeSeedById(int seedId) {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (seedIds[dx][dy] == seedId) {
					seedIds[dx][dy] = -1;
					return true;
				}
			}
		}
		return false;
	}

	// Add coin to house storage
	bool addCoin(int coinId) {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				// Check that this slot is empty (no food, seed, OR coin)
				if (foodIds[dx][dy] == -1 && seedIds[dx][dy] == -1 && coinIds[dx][dy] == -1) {
					coinIds[dx][dy] = coinId;
					return true;
				}
			}
		}
		return false;
	}

	// Get first coin ID from house storage
	int getFirstCoinId() const {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (coinIds[dx][dy] != -1) {
					return coinIds[dx][dy];
				}
			}
		}
		return -1;
	}

	// Remove coin from house storage
	bool removeCoinById(int coinId) {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (coinIds[dx][dy] == coinId) {
					coinIds[dx][dy] = -1;
					return true;
				}
			}
		}
		return false;
	}

	// Returns true if house has at least one coin item
	bool hasCoin() const {
		for (int dx = 0; dx < 3; ++dx)
			for (int dy = 0; dy < 3; ++dy)
				if (coinIds[dx][dy] != -1)
					return true;
		return false;
	}

	// Count how many coins are in house
	int countCoins() const {
		int count = 0;
		for (int dx = 0; dx < 3; ++dx)
			for (int dy = 0; dy < 3; ++dy)
				if (coinIds[dx][dy] != -1)
					count++;
		return count;
	}

	// Legacy methods for backward compatibility with old string-based system
	bool addItem(const std::string& itemType) {
		// For backward compatibility - not used in new system
		return false;
	}

	bool removeItem(const std::string& itemType) {
		// For backward compatibility - not used in new system
		return false;
	}

	bool hasItem(const std::string& itemType) const {
		// For backward compatibility - check if we have any food
		if (itemType == "food") {
			return hasFood();
		}
		return false;
	}
};

class HouseManager {
public:
    std::vector<House> houses;

    void addHouse(const House& s) { houses.push_back(s); }
    // Add more as needed
};

struct Farm {
	int ownerUnitId;
	int gridX, gridY; // Top-left of 3x3 area
	// 3x3 grid of seed/food IDs, -1 means empty
	int plantIds[3][3];
	// Time when each seed was planted (in SDL ticks), 0 means not planted
	Uint32 plantTimes[3][3];

	Farm(int ownerId, int x, int y)
		: ownerUnitId(ownerId), gridX(x), gridY(y) {
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j) {
				plantIds[i][j] = -1;
				plantTimes[i][j] = 0;
			}
	}

	// Returns true if seed was planted, false if full
	bool plantSeed(int seedId, Uint32 currentTime) {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (plantIds[dx][dy] == -1) {
					plantIds[dx][dy] = seedId;
					plantTimes[dx][dy] = currentTime;
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
				if (plantIds[dx][dy] == -1)
					return true;
		return false;
	}

	// Returns the first grown food ID found (seed that has been growing for 10+ seconds), -1 if not found
	int getFirstGrownFoodId(Uint32 currentTime) const {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (plantIds[dx][dy] != -1 && plantTimes[dx][dy] > 0) {
					// Check if 10 seconds (10000 ms) have passed
					if (currentTime - plantTimes[dx][dy] >= 10000) {
						return plantIds[dx][dy];
					}
				}
			}
		}
		return -1;
	}

	// Returns true if food was removed, false if not found
	bool removePlantById(int plantId) {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (plantIds[dx][dy] == plantId) {
					plantIds[dx][dy] = -1;
					plantTimes[dx][dy] = 0;
					return true;
				}
			}
		}
		return false;
	}
};

class FarmManager {
public:
    std::vector<Farm> farms;

    void addFarm(const Farm& f) { farms.push_back(f); }
    // Add more as needed
};

struct Market {
	int gridX, gridY; // Top-left of 3x3 area
	// 3x3 grid of food IDs being sold at each stall, -1 means empty
	int stallFoodIds[3][3];
	// 3x3 grid of seller unit IDs at each stall, -1 means no seller
	int stallSellerIds[3][3];
	// 3x3 grid of time when food was left at stall (in SDL ticks), 0 means seller present
	Uint32 stallAbandonTimes[3][3];

	Market(int x, int y)
		: gridX(x), gridY(y) {
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j) {
				stallFoodIds[i][j] = -1;
				stallSellerIds[i][j] = -1;
				stallAbandonTimes[i][j] = 0;
			}
	}

	// Returns true if there's at least one empty stall
	bool hasEmptyStall() const {
		for (int dx = 0; dx < 3; ++dx)
			for (int dy = 0; dy < 3; ++dy)
				if (stallFoodIds[dx][dy] == -1)
					return true;
		return false;
	}

	// Returns true if there's at least one stall with a seller
	bool hasActiveSeller() const {
		for (int dx = 0; dx < 3; ++dx)
			for (int dy = 0; dy < 3; ++dy)
				if (stallSellerIds[dx][dy] != -1)
					return true;
		return false;
	}

	// Find first empty stall (returns true if found, fills outX and outY with local stall coords 0-2)
	bool findEmptyStall(int& outX, int& outY) const {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (stallFoodIds[dx][dy] == -1) {
					outX = dx;
					outY = dy;
					return true;
				}
			}
		}
		return false;
	}

	// Find first stall with an active seller (returns true if found, fills outX and outY with local stall coords 0-2)
	bool findActiveSellerStall(int& outX, int& outY) const {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (stallSellerIds[dx][dy] != -1 && stallFoodIds[dx][dy] != -1) {
					outX = dx;
					outY = dy;
					return true;
				}
			}
		}
		return false;
	}
};

class MarketManager {
public:
    std::vector<Market> markets;

    void addMarket(const Market& m) { markets.push_back(m); }
    // Add more as needed
};

// Global house manager instance
extern HouseManager* g_HouseManager;

// Global farm manager instance
extern FarmManager* g_FarmManager;

// Global market manager instance
extern MarketManager* g_MarketManager;
