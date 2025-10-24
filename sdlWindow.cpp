#include "sdlWindow.h"
#include <SDL_ttf.h>
#include <iostream>

SDL_Window* startSdlWindow(SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init: %s\n", SDL_GetError());
        return nullptr;
    }
    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return nullptr;
    }
    SDL_Window* window = SDL_CreateWindow(
        "Ascii World",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        sdlWindowWidth, sdlWindowHeight,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return nullptr;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return nullptr;
    }
    return window;
}
