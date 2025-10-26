#include "Unit.h"
#include "CellGrid.h"
#include "Pathfinding.h"
#include <random>
#include <iostream>
#include <SDL.h>
#include <limits>
#include "Buildings.h"

StockpileManager* StockpileManager = nullptr;


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
        }

        // Pop the action once if the path is empty (either no path found or finished).
        // Defensive check to avoid popping an already-empty queue.
        if (path.empty()) {
            if (!actionQueue.empty()) {
                actionQueue.pop();
            }
        }
        break;
    }
	case ActionType::Eat: {
		// Check if at food location
		int gridX, gridY;
		cellGrid.pixelToGrid(x, y, gridX, gridY);

		// Find food at this location
		auto it = std::find_if(foods.begin(), foods.end(), [&](const Food& food) {
			int fx, fy;
			cellGrid.pixelToGrid(food.x, food.y, fx, fy);
			return fx == gridX && fy == gridY;
			});

		if (it != foods.end()) {
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
		// Build house: mark 3x3 as solid, add to StockpileManager
		for (int dx = 0; dx < 3; ++dx) {
			for (int dy = 0; dy < 3; ++dy) {
				MapCell* cell = cellGrid.getCell(houseGridX + dx, houseGridY + dy);
				if (cell) cell->isWalkable = false;
			}
		}
		if (g_StockpileManager) {
			g_StockpileManager->addStockpile(Stockpile(id, houseGridX, houseGridY));
		}
		std::cout << "Unit " << name << " built a house at (" << houseGridX << ", " << houseGridY << ")\n";
		actionQueue.pop();
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
        // Path to the food
        auto newPath = aStarFindPath(gridX, gridY, foodGridX, foodGridY, cellGrid);
        if (!newPath.empty()) {
            path = newPath;
            // Add Eat action with priority 9
            addAction(Action(ActionType::Eat, 9));
			addAction(Action(ActionType::BuildHouse, 8));
        }
    }
}