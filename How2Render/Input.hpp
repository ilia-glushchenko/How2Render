#pragma once

#include <SDL.h>
#include <glm/glm.hpp>

enum class eKeyState : uint8_t
{
	None = 0,
	Press,
	Repeate,
	Release,
};

struct InputEvents
{
	eKeyState keys[SDL_NUM_SCANCODES];
	glm::ivec2 mouse;
	bool quit;
};

InputEvents create_defalt_input_events()
{
	return InputEvents{ {}, {0, 0}, false };
}

void update_input(InputEvents &events)
{
	//This is a bit dumb but useful when you don't have events every frame
	for (uint32_t i = 0; i < SDL_NUM_SCANCODES; ++i)
	{
		if (events.keys[i] == eKeyState::Release)
		{
			events.keys[i] = eKeyState::None;
		}
		else if (events.keys[i] == eKeyState::Press)
		{
			events.keys[i] = eKeyState::Repeate;
		}
	}

	SDL_GetMouseState(&events.mouse.x, &events.mouse.y);

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			events.quit = true;
		}
		else
		{
			if (event.type == SDL_KEYDOWN)
			{
				events.keys[event.key.keysym.scancode] = eKeyState::Press;
			}
			else if (event.type == SDL_KEYUP)
			{
				events.keys[event.key.keysym.scancode] = eKeyState::Release;
			}
		}
	}
}

bool is_key_down(InputEvents& input_events, SDL_Scancode scancode)
{
	return input_events.keys[scancode] == eKeyState::Press || input_events.keys[scancode] == eKeyState::Repeate;
}
