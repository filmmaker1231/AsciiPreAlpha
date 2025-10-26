#pragma once
#include <string>

#include <vector>
#include <queue>
#include "Actions.h"
#include <SDL.h>
#include "Food.h"

class CellGrid; // Forward declaration

class Unit {
public:
	std::string name;   // Name of the unit
    int x, y;           // Position on the grid
    char symbol;        // Character to display (e.g., '@')
    int health;
	int hunger = 100;
	std::vector<std::string> inventory;
	int carriedFoodId = -1; // ID of food being carried, -1 if none
	int carriedSeedId = -1; // ID of seed being carried, -1 if none
	Uint32 lastHungerUpdate = 0;
	Uint32 lastHungerDebugPrint = 0;
    int id;
    unsigned int moveDelay;      // Delay in milliseconds between moves
    unsigned int lastMoveTime;   // Last time the unit moved (in SDL ticks)
	int houseGridX = -1; // Grid X of assigned house location
	int houseGridY = -1; // Grid Y of assigned house location



	std::vector<std::pair<int, int>> path;
	std::priority_queue<Action, std::vector<Action>, ActionComparator> actionQueue;

	  void addAction(const Action& action);
	  void processAction(CellGrid& cellGrid, std::vector<Food>& foods, std::vector<Seed>& seeds);
	  void tryFindAndPathToFood(CellGrid& cellGrid, std::vector<Food>& foods);
	  void bringItemToHouse(const std::string& itemType) {
		  addAction(Action(ActionType::BringItemToHouse, 5, itemType));
	  }
	  void eatFromHouse() {
		  addAction(Action(ActionType::EatFromHouse, 8));
	  }

	

	  Unit(int x, int y, char symbol, const std::string& name, int health = 100, int id = 0)
		  : x(x), y(y), symbol(symbol), name(name), health(health), hunger(100), lastHungerUpdate(0), lastHungerDebugPrint(0), id(id), moveDelay(200), lastMoveTime(0) {
	  }
};





