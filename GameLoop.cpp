#include "GameLoop.h"
#include "CellGrid.h"

void runMainLoop(SDL_Window* window, SDL_Renderer* renderer, CellGrid& cellGrid, bool& showCellGrid) {
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        renderCellGrid(renderer, cellGrid, showCellGrid);
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }
}
