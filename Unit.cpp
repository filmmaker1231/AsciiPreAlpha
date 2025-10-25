#include "Unit.h"
#include "CellGrid.h"
#include "Pathfinding.h"
#include <random>


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

void Unit::processAction(CellGrid& cellGrid) {
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

		// Move along the path if there is one
		if (!path.empty()) {
			auto [nextGridX, nextGridY] = path.front();
			int nextPixelX, nextPixelY;
			cellGrid.gridToPixel(nextGridX, nextGridY, nextPixelX, nextPixelY);
			x = nextPixelX;
			y = nextPixelY;
			path.erase(path.begin());
		}
		else {
			// No path found, pop the action to try again next frame
			actionQueue.pop();
		}

		// If path is now empty, pop the action to trigger a new wander next frame
		if (path.empty()) {
			actionQueue.pop();
		}
		break;
	}
	case ActionType::Eat:
		// ... eat logic ...
		break;
	// Add more cases as needed
	}
}