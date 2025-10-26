#pragma once
#include <vector>
#include <string>




struct House {
	int ownerUnitId;
	int gridX, gridY; // Top-left of 3x3 area
	// 3x3 grid of food IDs, -1 means empty
	int foodIds[3][3];

	House(int ownerId, int x, int y)
		: ownerUnitId(ownerId), gridX(x), gridY(y) {
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				foodIds[i][j] = -1;
	}

	// Returns true if food was added, false if full
	bool addFood(int foodId) {
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				if (foodIds[dx][dy] == -1) {
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
				if (foodIds[dx][dy] == -1)
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

// Global house manager instance
extern HouseManager* g_HouseManager;
