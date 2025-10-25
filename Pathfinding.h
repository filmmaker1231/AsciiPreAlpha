#pragma once
#include <vector>
#include <utility>
#include "CellGrid.h"

std::vector<std::pair<int, int>> aStarFindPath(
    int startX, int startY,
    int goalX, int goalY,
    const CellGrid& grid
);