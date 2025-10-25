#pragma once
#include <vector>
#include <unordered_map>
#include "sdlWindow.h"
#include "Food.h"



inline constexpr int GRID_SIZE = 40; // Choose your preferred size


struct MapCell {
    int gridX;  // Cell x coordinate (in grid units, not pixels)
    int gridY;  // Cell y coordinate (in grid units, not pixels)
    
    // References to objects in this cell
    std::vector<int> unitIds;           // IDs of units in this cell
    std::vector<int> foodIds;           // IDs of food items in this cell
    std::vector<int> seedIds;           // IDs of seeds in this cell
    std::vector<int> stockpileIds;      // IDs of stockpiles overlapping this cell
    
    // Tile information
    bool hasTile = false;               // Whether this cell has a placed tile
     
    
    // Pathfinding information
    bool isWalkable = true;             // Can units walk through this cell?
    
    MapCell() : gridX(0), gridY(0) {}
    
    MapCell(int x, int y) : gridX(x), gridY(y) {}
    
    // Clear all references
    void clear() {
        
        
        hasTile = false;
        isWalkable = true;
    }
    
    // Check if cell has any units
    bool hasUnits() const {
        return !unitIds.empty();
    }
    
    // Check if cell has any food
    bool hasFood() const {
        return !foodIds.empty();
    }
    
    // Check if cell has any seeds
    bool hasSeeds() const {
        return !seedIds.empty();
    }
    
    // Check if cell has any stockpiles
    bool hasStockpiles() const {
        return !stockpileIds.empty();
    }
};


// CellGrid manages a grid of MapCells for the entire game world

class CellGrid {
private:
    int widthInCells;   // Width of grid in cells
    int heightInCells;  // Height of grid in cells
    std::vector<MapCell> cells; // Flat array of cells (row-major order)
    
public:
	int getWidthInCells() const { return widthInCells; }
    int getHeightInCells() const { return heightInCells; }
	int getWidthInPixels() const { return widthInCells * GRID_SIZE; }
    int getHeightInPixels() const { return heightInCells * GRID_SIZE; }


    CellGrid(int widthInPixels, int heightInPixels) {
        widthInCells = (widthInPixels + GRID_SIZE - 1) / GRID_SIZE;
        heightInCells = (heightInPixels + GRID_SIZE - 1) / GRID_SIZE;
        cells.resize(widthInCells * heightInCells);
        
        // Initialize cell coordinates
        for (int y = 0; y < heightInCells; y++) {
            for (int x = 0; x < widthInCells; x++) {
                int index = y * widthInCells + x;
                cells[index] = MapCell(x, y);
            }
        }
    }
    
    // Get cell at grid coordinates (in cells, not pixels)
    MapCell* getCell(int gridX, int gridY) {
        if (gridX < 0 || gridX >= widthInCells || gridY < 0 || gridY >= heightInCells) {
            return nullptr;
        }
        return &cells[gridY * widthInCells + gridX];
    }
    
    // Get cell at pixel coordinates
    MapCell* getCellAtPixel(int pixelX, int pixelY) {
        int gridX = pixelX / GRID_SIZE;
        int gridY = pixelY / GRID_SIZE;
        return getCell(gridX, gridY);
    }
    
    // Convert pixel coordinates to grid coordinates
    void pixelToGrid(int pixelX, int pixelY, int& gridX, int& gridY) const {
        gridX = pixelX / GRID_SIZE;
        gridY = pixelY / GRID_SIZE;
    }
    
    // Convert grid coordinates to pixel coordinates (top-left corner of cell)
    void gridToPixel(int gridX, int gridY, int& pixelX, int& pixelY) const {
        pixelX = gridX * GRID_SIZE;
        pixelY = gridY * GRID_SIZE;
    }
    
    // Clear all cells
    void clearAll() {
        for (auto& cell : cells) {
            cell.clear();
        }
    }
    
	
    
    
    // Check if a cell is walkable (for pathfinding)
    bool isCellWalkable(int gridX, int gridY) const {
        if (gridX < 0 || gridX >= widthInCells || gridY < 0 || gridY >= heightInCells) {
            return false;
        }
        const MapCell& cell = cells[gridY * widthInCells + gridX];
        return cell.isWalkable;
    }
};

void renderCellGrid(SDL_Renderer* renderer, const CellGrid& cellGrid, bool showCellInfo = false);

