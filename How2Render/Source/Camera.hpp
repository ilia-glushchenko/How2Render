#include "Window.hpp"
#include "Input.hpp"
#include "Helpers.hpp"

#include <glm/glm.hpp>

struct Camera
{
	glm::mat4 view;
	glm::mat4 model;

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;

	float yaw;
	float pitch;

	float speed;
	bool capture_mouse;
};

inline Camera CreateDefaultCamera()
{
	return Camera{
		glm::mat4(1), //view
		glm::mat4(1), //model

		glm::vec3{0, 0, 0},	 //position
		glm::vec3{0, 0,-1}, //forward
		glm::vec3{0, 1, 0},	 //up

		0, //yaw
		0, //pitch

		0.001f, //speed
		false //capture mouse
	};
}

inline bool UpdateCamera(Camera& camera, InputEvents const& inputEvents, Window const& window)
{
	bool moved = false;

	//Update relative mouse mode
	if (inputEvents.keys[SDL_SCANCODE_F] == eKeyState::Press)
	{
		camera.capture_mouse = !camera.capture_mouse;
		SDL_ShowCursor(!camera.capture_mouse);
	}

	glm::vec4 const world_forward = glm::vec4(0., 0., -1., 0.);
	glm::vec4 const world_right = glm::vec4(1., 0., 0., 0.);

	glm::vec4 forward = glm::vec4(0);
	glm::vec4 right = glm::vec4(0);
	glm::mat4 cameraMatrix = help::CreateCameraMatrix(camera.position, camera.yaw, camera.pitch);

	if (IsKeyDown(inputEvents, SDL_SCANCODE_W))
	{
		forward = cameraMatrix * world_forward * camera.speed;
		moved = true;
	}
	if (IsKeyDown(inputEvents, SDL_SCANCODE_S))
	{
		forward = cameraMatrix * -world_forward * camera.speed;
		moved = true;
	}
	if (IsKeyDown(inputEvents, SDL_SCANCODE_A))
	{
		right = cameraMatrix * -world_right * camera.speed;
		moved = true;
	}
	if (IsKeyDown(inputEvents, SDL_SCANCODE_D))
	{
		right = cameraMatrix * world_right * camera.speed;
		moved = true;
	}
	if (IsKeyDown(inputEvents, SDL_SCANCODE_Q))
	{
		camera.position.y -= camera.speed;
		moved = true;
	}
	if (IsKeyDown(inputEvents, SDL_SCANCODE_E))
	{
		camera.position.y += camera.speed;
		moved = true;
	}

	glm::ivec2 const windowSize = GetWindowSize(window);
	if (camera.capture_mouse)
	{
		glm::ivec2 const windowCenter = windowSize / 2;
		glm::vec2 const mouseDelta = windowCenter - inputEvents.mouse;
		SDL_WarpMouseInWindow(window.window, windowCenter.x, windowCenter.y);

		camera.pitch += mouseDelta.y / windowSize.x;
		camera.yaw += mouseDelta.x / windowSize.y;

		if (glm::length2(mouseDelta) > 0)
		{
			moved = true;
		}
	}

	camera.position.x += forward.x + right.x;
	camera.position.y += forward.y + right.y;
	camera.position.z += forward.z + right.z;
	camera.view = help::CreateViewMatrix(camera.position, camera.yaw, camera.pitch);

	return moved;
}
