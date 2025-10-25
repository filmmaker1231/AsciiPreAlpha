#include "PathClick.h"


void pathClick(sdl& app) {

	if (app.unitManager) {
		auto& units = const_cast<std::vector<Unit>&>(app.unitManager->getUnits());
		for (auto& unit : units) {
			if (!unit.path.empty()) {
				// Move to the next cell in the path
				auto [nextGridX, nextGridY] = unit.path.front();
				// Convert grid to pixel coordinates (top-left of cell)
				int nextPixelX, nextPixelY;
				app.cellGrid->gridToPixel(nextGridX, nextGridY, nextPixelX, nextPixelY);
				unit.x = nextPixelX;
				unit.y = nextPixelY;
				unit.path.erase(unit.path.begin());
			}
		}
	}

}
