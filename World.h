#pragma once


// Map is 48x25 tiles
// Region represents a 48x25 tile area in the world
// The world is composed of 9 regions arranged in a 3x3 grid
struct Region {
    int id;           // Region identifier (0-8 for 3x3 grid)
    int gridX;        // X position in the region grid (0-2)
    int gridY;        // Y position in the region grid (0-2)
    int worldX;       // Starting X coordinate in world space (gridX * 200)
    int worldY;       // Starting Y coordinate in world space (gridY * 200)
    int width;        // Region width in tiles (200)
    int height;       // Region height in tiles (200)

    // Constructor for easy initialization
    Region(int regionId, int gx, int gy) 
        : id(regionId), gridX(gx), gridY(gy), 
          worldX(gx * 48), worldY(gy * 25),
          width(48), height(25) {}

    // Default constructor
    Region() : id(0), gridX(0), gridY(0), worldX(0), worldY(0), width(48), height(25) {}

    // Check if a world position is within this region
    bool containsPosition(int x, int y) const {
        return x >= worldX && x < worldX + width &&
               y >= worldY && y < worldY + height;
    }
};