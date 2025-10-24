// EXAMPLE_USAGE.cpp
// This file demonstrates how to use the optimized unit update system
// It is not compiled into the main project but serves as documentation

/*

BASIC USAGE EXAMPLE
===================

#include "UpdateScheduler.h"
#include "SpatialGrid.h"
#include "OptimizedUnitUpdate.h"

void gameLoop() {
    // 1. Initialize the update scheduler
    // Slice count of 4 means each unit's AI updates every 4 frames
    UpdateScheduler scheduler(4);
    
    // 2. Your game entities
    std::vector<Unit> units;
    std::vector<Food> foods;
    std::vector<Stockpile> stockpiles;
    std::unordered_map<GridKey, PlacedTile> placedTiles;
    
    // 3. Game loop
    while (running) {
        // Handle input, update hunger, food spawning, etc.
        
        // Use the optimized update system
        // This handles both staggered AI updates and per-frame movement
        updateUnitsOptimized(
            units,           // Your unit vector
            scheduler,       // The update scheduler
            foods,           // Food in the world
            stockpiles,      // Stockpile locations
            placedTiles,     // Obstacles/world tiles
            gridWidth,       // World bounds
            gridHeight
        );
        
        // Render with interpolation for smooth motion
        // Pass both unit font and stat font (can be same or different sizes)
        renderUnitsInterpolated(renderer, font, font, units, 1.0f);
    }
}


SPATIAL GRID USAGE EXAMPLE
===========================

void useSpatialGridExample() {
    std::vector<Unit> units = getUnits();
    
    // Build spatial grid from current unit positions
    SpatialGrid grid = buildSpatialGrid(units);
    
    // Example 1: Find units near a specific position
    int targetX = 500, targetY = 300;
    int searchRadius = 200;
    std::vector<int> nearbyUnitIds = grid.getUnitsInRadius(
        targetX, targetY, 0, searchRadius
    );
    
    // Example 2: Check for units in a specific cell
    const std::vector<int>& cellUnits = grid.getUnitsInCell(
        targetX, targetY, 0
    );
    
    // Example 3: Use for collision detection
    for (const auto& projectile : projectiles) {
        auto nearbyUnits = grid.getUnitsInRadius(
            projectile.x, projectile.y, projectile.layer, 50
        );
        
        for (int unitId : nearbyUnits) {
            // Check precise collision with each nearby unit
            // Instead of checking all units in the world!
        }
    }
}


CUSTOM AI UPDATE EXAMPLE
=========================

// You can implement custom AI logic by following this pattern:

void performCustomAIUpdate(
    Unit& unit,
    const std::vector<Enemy>& enemies,
    const GameState& gameState
) {
    // This function is called only for units in the current slice
    // Perform expensive operations here:
    
    // 1. Pathfinding
    if (unit.needsNewPath) {
        unit.path = calculateExpensivePath(unit.position, unit.goal);
    }
    
    // 2. Decision-making
    if (unit.path.empty()) {
        Enemy* nearestEnemy = findNearestEnemy(unit, enemies);
        if (nearestEnemy) {
            unit.goal = nearestEnemy->position;
        }
    }
    
    // 3. Complex state updates
    unit.updateStateMachine();
    unit.recalculateStats();
}

void performCustomMovement(
    Unit& unit,
    const std::unordered_map<GridKey, PlacedTile>& obstacles
) {
    // This function is called EVERY frame for ALL units
    // Keep it lightweight!
    
    // Follow the current path or direction
    if (!unit.path.empty()) {
        GridKey nextWaypoint = unit.path.front();
        
        // Store previous position for interpolation
        unit.prevX = unit.x;
        unit.prevY = unit.y;
        
        // Move to next position
        unit.x = nextWaypoint.x * GRID_SIZE;
        unit.y = nextWaypoint.y * GRID_SIZE;
        
        unit.path.erase(unit.path.begin());
    }
}


ADJUSTING PERFORMANCE PARAMETERS
==================================

// Tuning slice count for different scenarios:

void lowUnitCount_HighResponsiveness() {
    // For ~100 units where AI responsiveness is critical
    UpdateScheduler scheduler(2);  // AI updates every 2 frames
    // Trade-off: Higher CPU usage per frame
}

void mediumUnitCount_Balanced() {
    // For ~500 units with good balance
    UpdateScheduler scheduler(4);  // AI updates every 4 frames
    // Trade-off: Balanced performance and responsiveness
}

void highUnitCount_MaxPerformance() {
    // For 1000+ units where performance is critical
    UpdateScheduler scheduler(8);  // AI updates every 8 frames
    // Trade-off: AI feels less responsive but handles more units
}

void veryHighUnitCount_ReducedAI() {
    // For 5000+ units in large battles
    UpdateScheduler scheduler(16); // AI updates every 16 frames
    // Trade-off: AI updates rarely but can handle massive unit counts
}


INTERPOLATION EXAMPLES
=======================

void renderWithFixedTimestep() {
    // For games using fixed timestep with accumulator
    
    const float FIXED_DT = 1.0f / 60.0f;
    float accumulator = 0.0f;
    
    while (running) {
        float frameTime = getFrameTime();
        accumulator += frameTime;
        
        // Update in fixed steps
        while (accumulator >= FIXED_DT) {
            updateUnitsOptimized(...);
            accumulator -= FIXED_DT;
        }
        
        // Render with sub-frame interpolation
        float alpha = accumulator / FIXED_DT;
        renderUnitsInterpolated(renderer, font, font, units, alpha);
    }
}

void renderSimple() {
    // For games with variable timestep (current implementation)
    
    while (running) {
        updateUnitsOptimized(...);
        
        // Alpha = 1.0 means use current position
        renderUnitsInterpolated(renderer, font, font, units, 1.0f);
    }
}


PROFILING EXAMPLE
==================

void profileUpdatePerformance() {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    updateUnitsOptimized(units, scheduler, foods, stockpiles, 
                        placedTiles, gridWidth, gridHeight);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime
    );
    
    std::cout << "Update took: " << duration.count() << " microseconds\n";
    std::cout << "Units: " << units.size() << "\n";
    std::cout << "Microseconds per unit: " 
              << (float)duration.count() / units.size() << "\n";
}


COMMON PITFALLS
================

// DON'T do this - defeats the purpose of optimization:
void badExample() {
    // This updates ALL units' AI every frame!
    for (auto& unit : units) {
        performUnitAIUpdate(unit, ...);  // BAD: Too expensive
    }
}

// DO this - use the optimized system:
void goodExample() {
    // This automatically handles staggered updates
    updateUnitsOptimized(units, scheduler, ...);  // GOOD: Optimized
}


// DON'T do this - ignores interpolation:
void badRenderExample() {
    for (const auto& unit : units) {
        // Always renders at discrete grid positions
        renderAt(unit.x, unit.y);  // BAD: Choppy movement
    }
}

// DO this - use interpolated positions:
void goodRenderExample() {
    renderUnitsInterpolated(renderer, font, font, units, 1.0f);  // GOOD: Smooth
}

*/
