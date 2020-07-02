#pragma once

#include "Math.hpp"
#include < SDL.h>
#include <cstdio>

struct Window
{
	SDL_Window *window = nullptr;
	SDL_Surface *surface = nullptr;
};

inline Window CreateNewWindow(uint32_t width, uint32_t height)
{
	Window window = {nullptr, nullptr};

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
	{
		fprintf(stderr, "Could not initialize sdl2: %s\n", SDL_GetError());
		return window;
	}

	window.window = SDL_CreateWindow(
		"How2Render",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_SHOWN);
	if (window.window == nullptr)
	{
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
		return {nullptr, nullptr};
	}

	//Get windows surface and fill it with white color
	window.surface = SDL_GetWindowSurface(window.window);
	SDL_FillRect(window.surface, nullptr, SDL_MapRGB(window.surface->format, 0xFF, 0xFF, 0xFF));

	return window;
}

inline void DestroyWindow(Window window)
{
	SDL_DestroyWindow(window.window);
	SDL_Quit();
}

inline XMINT2 GetWindowSize(Window const &window)
{
	XMINT2 result;
	SDL_GetWindowSize(window.window, &result.x, &result.y);
	return result;
}