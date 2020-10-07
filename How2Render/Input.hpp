#pragma once

#include <DirectXMath.h>
#include <SDL.h>

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
	DirectX::XMINT2 mouse;
	bool quit;
};

InputEvents CreateDefaultInputEvents()
{
	return InputEvents{{}, {0, 0}, false};
}

void UpdateInput(InputEvents &events)
{
	//Since we don't get the events every frame
	//We need to store the key states, between those frames
	//The simplest way is to put them into the custom array.
	//Fortunately enough SDL already provides enum for arrays like that.
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