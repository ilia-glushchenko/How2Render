#pragma once

#include "Math.hpp"
#include <SDL.h>

namespace h2r
{

	enum class eKeyState : uint8_t
	{
		None = 0,
		Press,
		Repeat,
		Release,
	};

	struct InputEvents
	{
		eKeyState keys[SDL_NUM_SCANCODES];
		XMINT2 mouse;
		bool quit;
	};

	inline InputEvents CreateDefaultInputEvents()
	{
		return InputEvents{{}, {0, 0}, false};
	}

	inline void UpdateInput(InputEvents &events)
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
				events.keys[i] = eKeyState::Repeat;
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

	inline bool IsKeyPressed(InputEvents const &inputEvents, SDL_Scancode scancode)
	{
		return inputEvents.keys[scancode] == eKeyState::Press;
	}

	inline bool IsKeyDown(InputEvents const &inputEvents, SDL_Scancode scancode)
	{
		return inputEvents.keys[scancode] == eKeyState::Press || inputEvents.keys[scancode] == eKeyState::Repeat;
	}

} // namespace h2r
