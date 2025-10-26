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

	House(int ownerId, int x, int y)
		: ownerUnitId(ownerId), gridX(x), gridY(y) {
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j) {
				foodIds[i][j] = -1;
				seedIds[i][j] = -1;
			}
	}

	// Returns true if food was added, false if full
	bool addFood(int foodId) {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				// Check that this slot is empty (no food AND no seed)
				if (foodIds[dx][dy] == -1 && seedIds[dx][dy] == -1) {
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
				// Empty slot must have both food AND seed as -1
				if (foodIds[dx][dy] == -1 && seedIds[dx][dy] == -1)
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
				// Check that this slot is empty (no food AND no seed)
				if (foodIds[dx][dy] == -1 && seedIds[dx][dy] == -1) {
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

// Global house manager instance
extern HouseManager* g_HouseManager;

// Global farm manager instance
extern FarmManager* g_FarmManager;
