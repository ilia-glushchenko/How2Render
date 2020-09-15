#pragma once

#include "Math.hpp"
#include <SDL.h>
#include <cstdio>

namespace h2r
{

	struct Window
	{
		SDL_Window *pWindow = nullptr;
		SDL_Surface *pSurface = nullptr;
	};

	inline Window CreateNewWindow(uint32_t width, uint32_t height)
	{
		Window window{nullptr, nullptr};

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
		{
			fprintf(stderr, "Could not initialize sdl2: %s\n", SDL_GetError());
			return window;
		}

		window.pWindow = SDL_CreateWindow(
			"How2Render",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			width, height,
			SDL_WINDOW_SHOWN);
		if (window.pWindow == nullptr)
		{
			fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
			return window;
		}

		window.pSurface = SDL_GetWindowSurface(window.pWindow);
		SDL_FillRect(window.pSurface, nullptr, SDL_MapRGB(window.pSurface->format, 0xFF, 0xFF, 0xFF));

		return window;
	}

	inline void CleanupWindow(Window window)
	{
		SDL_Delay(2000);
		SDL_DestroyWindow(window.pWindow);
		SDL_Quit();
	}

	inline XMINT2 GetWindowSize(Window const &window)
	{
		XMINT2 result;
		SDL_GetWindowSize(window.pWindow, &result.x, &result.y);
		return result;
	}

} // namespace h2r
