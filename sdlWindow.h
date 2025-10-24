#pragma once
#include <SDL.h>

inline constexpr int sdlWindowWidth = 1920;
inline constexpr int sdlWindowHeight = 1000;

// Returns a new SDL_Window* and sets renderer. Returns nullptr on failure.
SDL_Window* startSdlWindow(SDL_Renderer*& renderer);

