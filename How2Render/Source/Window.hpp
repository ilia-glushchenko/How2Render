#pragma once

#include <DirectXMath.h>
#include <SDL.h>
#include <stdio.h>

struct Window
{
	SDL_Window *window;
	SDL_Surface *surface;
};

inline Window CreateNewWindow(uint32_t width, uint32_t height)
{
	Window window{ nullptr, nullptr };
	
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
		return window;
	}

	window.surface = SDL_GetWindowSurface(window.window);
	SDL_FillRect(window.surface, nullptr, SDL_MapRGB(window.surface->format, 0xFF, 0xFF, 0xFF));

	return window;
}

inline void DestroyWindow(Window window)
{
	SDL_Delay(2000);
	SDL_DestroyWindow(window.window);
	SDL_Quit();
}

inline DirectX::XMINT2 GetWindowSize(Window const& window)
{
	DirectX::XMINT2 result;
	SDL_GetWindowSize(window.window, &result.x, &result.y);
	return result;
}