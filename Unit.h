#pragma once
#include <string>
#include <vector>
#include <queue>
#include "Actions.h"

class CellGrid; // Forward declaration

class Unit {
public:
	std::string name;   // Name of the unit
    int x, y;           // Position on the grid
    char symbol;        // Character to display (e.g., '@')
    int health;
    int id;



	std::vector<std::pair<int, int>> path;
	std::priority_queue<Action, std::vector<Action>, ActionComparator> actionQueue;

	  void addAction(const Action& action);
	  void processAction(CellGrid& cellGrid);


	

     Unit(int x, int y, char symbol, const std::string& name, int health = 100, int id = 0)
		 : x(x), y(y), symbol(symbol), name(name), health(health), id(id) {
	 }
};
