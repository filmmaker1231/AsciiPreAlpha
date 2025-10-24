#include "CellGrid.h"

#include <SDL.h>

void renderCellGrid(SDL_Renderer* renderer, const CellGrid& cellGrid, bool showCellInfo) {
    // Draw grid lines with semi-transparent white
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 80);
    
    int widthInPixels = cellGrid.getWidthInPixels();
    int heightInPixels = cellGrid.getHeightInPixels();
    int widthInCells = cellGrid.getWidthInCells();
    int heightInCells = cellGrid.getHeightInCells();
    
    // Draw vertical lines
    for (int x = 0; x <= widthInCells; x++) {
        int pixelX = x * GRID_SIZE;
        SDL_RenderDrawLine(renderer, pixelX, 0, pixelX, heightInPixels);
    }
    
    // Draw horizontal lines
    for (int y = 0; y <= heightInCells; y++) {
        int pixelY = y * GRID_SIZE;
        SDL_RenderDrawLine(renderer, 0, pixelY, widthInPixels, pixelY);
    }
    
    // Optionally highlight cells with data
    if (showCellInfo) {
        for (int y = 0; y < heightInCells; y++) {
            for (int x = 0; x < widthInCells; x++) {
                const MapCell* cell = const_cast<CellGrid&>(cellGrid).getCell(x, y);
                if (cell) {
                    int pixelX = x * GRID_SIZE;
                    int pixelY = y * GRID_SIZE;
                    
                    // Highlight cells with units (green tint)
                    if (cell->hasUnits()) {
                        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 30);
                        SDL_Rect rect = {pixelX, pixelY, GRID_SIZE, GRID_SIZE};
                        SDL_RenderFillRect(renderer, &rect);
                    }
                    
                    // Highlight cells with food (yellow tint)
                    if (cell->hasFood()) {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 30);
                        SDL_Rect rect = {pixelX, pixelY, GRID_SIZE, GRID_SIZE};
                        SDL_RenderFillRect(renderer, &rect);
                    }
                    
                    // Highlight cells with seeds (orange tint)
                    if (cell->hasSeeds()) {
                        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 30);
                        SDL_Rect rect = {pixelX, pixelY, GRID_SIZE, GRID_SIZE};
                        SDL_RenderFillRect(renderer, &rect);
                    }
                    
                    // Highlight non-walkable cells (red tint)
                    if (!cell->isWalkable) {
                        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 30);
                        SDL_Rect rect = {pixelX, pixelY, GRID_SIZE, GRID_SIZE};
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
            }
        }
    }
}
