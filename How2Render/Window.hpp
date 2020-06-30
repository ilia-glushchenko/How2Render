#pragma once

#include <SDL.h>
#include <stdio.h>

struct Window
{
	SDL_Window *window;
	SDL_Surface *surface;
};

Window create_window(uint32_t width, uint32_t height)
{
	Window window;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
		return {nullptr, nullptr};
	}
	window.window = SDL_CreateWindow(
		"How2Render",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_SHOWN);

	if (window.window == nullptr)
	{
		fprintf(stderr, "could not create window: %s\n", SDL_GetError());
		return {nullptr, nullptr};
	}
	window.surface = SDL_GetWindowSurface(window.window);
	SDL_FillRect(window.surface, nullptr, SDL_MapRGB(window.surface->format, 0xFF, 0xFF, 0xFF));

	return window;
}

glm::ivec2 get_window_size(Window const &window)
{
	int32_t screen_height = 0;
	int32_t screen_width = 0;
	SDL_GetWindowSize(window.window, &screen_width, &screen_height);
	return {screen_width, screen_height};
}

void destoroy_window(Window window)
{
	SDL_Delay(2000);
	SDL_DestroyWindow(window.window);
	SDL_Quit();
}