/*/ CellGridExample.cpp
// Example demonstrations of using the CellGrid system
// This file is NOT compiled - it's for reference only

//#include "CellGrid.h"
//#include "Pathfinder.h"
//#include "Unit.h"
//#include "Food.h"
//#include "Seed.h"
//#include "Stockpile.h"

// Example 1: Initialize and update the cell grid
void example1_InitializeAndUpdate() {
    // Create cell grid (typically done once at game start)
    int screenWidth = 800;
    int screenHeight = 600;
    CellGrid cellGrid(screenWidth, screenHeight);
    
    // Assume we have these from the game state
    std::unordered_map<int, Unit> units;
    std::vector<Food> foods;
    std::vector<Seed> seeds;
    std::vector<Stockpile> stockpiles;
    std::unordered_map<GridKey, PlacedTile> placedTiles;
    
    // Update all cells with current game state (do this every frame)
    cellGrid.updateAll(units, foods, seeds, stockpiles, placedTiles);
}

// Example 2: Find nearest food using cell grid (O(r²) instead of O(n))
void example2_FindNearestFood(
    Unit& unit,
    const std::vector<Food>& foods,
    CellGrid& cellGrid,
    int gridWidth,
    int gridHeight
) {
    int foodIndex, foodX, foodY;
    
    // Search within 10 cells radius (400 pixels)
    bool found = cellGrid.findNearestFood(
        unit.x, unit.y,           // Unit's position
        foodIndex, foodX, foodY,  // Output: found food info
        foods,                    // Reference to food vector
        10                        // Search radius in cells
    );
    
    if (found) {
        // Create path to the food
        int ux = unit.x / GRID_SIZE;
        int uy = unit.y / GRID_SIZE;
        GridKey start{ux, uy, unit.layer};
        GridKey goal{foodX / GRID_SIZE, foodY / GRID_SIZE, unit.layer};
        
        // Use normal pathfinding
        std::unordered_map<GridKey, PlacedTile> placedTiles; // from game state
        auto path = findPath(start, goal, placedTiles, gridWidth / GRID_SIZE, gridHeight / GRID_SIZE);
        
        if (!path.empty()) {
            unit.path = path;
            // Remove start position if needed
            if (!unit.path.empty() && unit.path.front() == start) {
                unit.path.erase(unit.path.begin());
            }
        }
    }
}

// Example 3: Use cell-based pathfinding (faster walkability checks)
void example3_CellBasedPathfinding(
    Unit& unit,
    int targetX,
    int targetY,
    CellGrid& cellGrid,
    int gridWidth,
    int gridHeight
) {
    int ux = unit.x / GRID_SIZE;
    int uy = unit.y / GRID_SIZE;
    int tx = targetX / GRID_SIZE;
    int ty = targetY / GRID_SIZE;
    
    GridKey start{ux, uy, unit.layer};
    GridKey goal{tx, ty, unit.layer};
    
    // Use the cell grid for pathfinding (faster)
    auto path = findPathWithCellGrid(
        start, goal,
        &cellGrid,
        gridWidth / GRID_SIZE,
        gridHeight / GRID_SIZE
    );
    
    if (!path.empty()) {
        unit.path = path;
        if (!unit.path.empty() && unit.path.front() == start) {
            unit.path.erase(unit.path.begin());
        }
    }
}

// Example 4: Check what's at a specific location
void example4_QueryCellContents(
    int pixelX,
    int pixelY,
    CellGrid& cellGrid
) {
    MapCell* cell = cellGrid.getCellAtPixel(pixelX, pixelY);
    
    if (cell) {
        // Check cell contents
        if (cell->hasUnits()) {
            std::cout << "Cell has " << cell->unitIds.size() << " units:\n";
            for (int unitId : cell->unitIds) {
                std::cout << "  - Unit ID: " << unitId << "\n";
            }
        }
        
        if (cell->hasFood()) {
            std::cout << "Cell has " << cell->foodIds.size() << " food items\n";
        }
        
        if (cell->hasSeeds()) {
            std::cout << "Cell has " << cell->seedIds.size() << " seeds\n";
        }
        
        if (!cell->isWalkable) {
            std::cout << "Cell is blocked (wall or obstacle)\n";
        }
        
        if (cell->hasTile && cell->tilePtr) {
            std::cout << "Cell has a tile at layer " << cell->tilePtr->layer << "\n";
        }
    }
}

// Example 5: Find all units in a rectangular area
void example5_FindUnitsInArea(
    int topLeftX,
    int topLeftY,
    int width,
    int height,
    CellGrid& cellGrid,
    std::vector<int>& outUnitIds
) {
    outUnitIds.clear();
    
    // Convert to grid coordinates
    int startGridX = topLeftX / GRID_SIZE;
    int startGridY = topLeftY / GRID_SIZE;
    int endGridX = (topLeftX + width) / GRID_SIZE;
    int endGridY = (topLeftY + height) / GRID_SIZE;
    
    // Iterate through cells in the area
    for (int y = startGridY; y <= endGridY; y++) {
        for (int x = startGridX; x <= endGridX; x++) {
            MapCell* cell = cellGrid.getCell(x, y);
            if (cell && cell->hasUnits()) {
                // Add all units in this cell
                for (int unitId : cell->unitIds) {
                    outUnitIds.push_back(unitId);
                }
            }
        }
    }
}

// Example 6: Find nearest walkable cell
void example6_FindNearestWalkableCell(
    int pixelX,
    int pixelY,
    CellGrid& cellGrid,
    int& outGridX,
    int& outGridY
) {
    int startGridX, startGridY;
    cellGrid.pixelToGrid(pixelX, pixelY, startGridX, startGridY);
    
    // Check if current cell is walkable
    if (cellGrid.isCellWalkable(startGridX, startGridY)) {
        outGridX = startGridX;
        outGridY = startGridY;
        return;
    }
    
    // Search in expanding squares
    for (int radius = 1; radius <= 10; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                // Only check perimeter
                if (radius > 1 && std::abs(dx) != radius && std::abs(dy) != radius) {
                    continue;
                }
                
                int checkX = startGridX + dx;
                int checkY = startGridY + dy;
                
                if (cellGrid.isCellWalkable(checkX, checkY)) {
                    outGridX = checkX;
                    outGridY = checkY;
                    return;
                }
            }
        }
    }
    
    // No walkable cell found
    outGridX = startGridX;
    outGridY = startGridY;
}

// Example 7: Optimized food collection AI using cell grid
void example7_OptimizedFoodCollectionAI(
    Unit& unit,
    const std::vector<Food>& foods,
    const std::vector<Stockpile>& stockpiles,
    CellGrid& cellGrid,
    int gridWidth,
    int gridHeight
) {
    // Check if unit's stockpile has space
    bool hasSpace = false;
    const Stockpile* homeStockpile = nullptr;
    
    for (const auto& sp : stockpiles) {
        if (sp.ownerUnitId == unit.id) {
            homeStockpile = &sp;
            // Check if any tile has no food
            hasSpace = std::any_of(sp.tiles.begin(), sp.tiles.end(),
                [](const StockpileTile& t) { return !t.hasFood; });
            break;
        }
    }
    
    if (!hasSpace || !homeStockpile) {
        return; // No stockpile or no space
    }
    
    // Use cell grid to find nearest food (fast)
    int foodIndex, foodX, foodY;
    if (cellGrid.findNearestFood(unit.x, unit.y, foodIndex, foodX, foodY, foods, 15)) {
        // Found food within 15 cells (600 pixels)
        int ux = unit.x / GRID_SIZE;
        int uy = unit.y / GRID_SIZE;
        GridKey start{ux, uy, unit.layer};
        GridKey goal{foodX / GRID_SIZE, foodY / GRID_SIZE, unit.layer};
        
        // Use cell-based pathfinding (faster)
        auto path = findPathWithCellGrid(
            start, goal,
            &cellGrid,
            gridWidth / GRID_SIZE,
            gridHeight / GRID_SIZE
        );
        
        if (!path.empty()) {
            unit.path = path;
            if (!unit.path.empty() && unit.path.front() == start) {
                unit.path.erase(unit.path.begin());
            }
            std::cout << "Unit " << unit.id << " found food at distance "
                      << path.size() << " cells\n";
        }
    }
}

// Example 8: Check if position is safe (no thieves nearby)
bool example8_IsPositionSafe(
    int pixelX,
    int pixelY,
    CellGrid& cellGrid,
    const std::unordered_map<int, Unit>& units,
    int searchRadius = 3
) {
    int startGridX, startGridY;
    cellGrid.pixelToGrid(pixelX, pixelY, startGridX, startGridY);
    
    // Search nearby cells for thieves (low morality units)
    for (int dy = -searchRadius; dy <= searchRadius; dy++) {
        for (int dx = -searchRadius; dx <= searchRadius; dx++) {
            MapCell* cell = cellGrid.getCell(startGridX + dx, startGridY + dy);
            if (cell && cell->hasUnits()) {
                for (int unitId : cell->unitIds) {
                    auto it = units.find(unitId);
                    if (it != units.end() && it->second.morality < 30.0f) {
                        // Found a thief nearby!
                        return false;
                    }
                }
            }
        }
    }
    
    return true; // No thieves found
}

// Example 9: Convert between coordinate systems
void example9_CoordinateConversion(CellGrid& cellGrid) {
    // Pixel to grid
    int pixelX = 240;
    int pixelY = 160;
    int gridX, gridY;
    cellGrid.pixelToGrid(pixelX, pixelY, gridX, gridY);
    std::cout << "Pixel (" << pixelX << ", " << pixelY << ") is in grid cell (" 
              << gridX << ", " << gridY << ")\n";
    
    // Grid to pixel (top-left corner of cell)
    int backToPixelX, backToPixelY;
    cellGrid.gridToPixel(gridX, gridY, backToPixelX, backToPixelY);
    std::cout << "Grid cell (" << gridX << ", " << gridY << ") starts at pixel (" 
              << backToPixelX << ", " << backToPixelY << ")\n";
}

// Example 10: Performance comparison
void example10_PerformanceComparison(
    const Unit& unit,
    const std::vector<Food>& foods,
    CellGrid& cellGrid
) {
    // Old method: O(n) - iterate through all food
    float minDist = 1e9f;
    int closestFoodIndex = -1;
    for (size_t i = 0; i < foods.size(); i++) {
        float dx = foods[i].x - unit.x;
        float dy = foods[i].y - unit.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < minDist) {
            minDist = dist;
            closestFoodIndex = i;
        }
    }
    
    // New method: O(r²) - search in expanding squares
    int foodIndex, foodX, foodY;
    bool found = cellGrid.findNearestFood(
        unit.x, unit.y,
        foodIndex, foodX, foodY,
        foods,
        10 // Only check within 10 cells
    );
    
    // With 1000 food items and typical search radius:
    // Old: 1000 distance calculations
    // New: ~100-400 cell checks (10x10 to 20x20 area)
    // Speedup: 2.5x - 10x faster!
}
*/