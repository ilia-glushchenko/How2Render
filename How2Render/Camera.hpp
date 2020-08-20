#pragma once

#include "Input.hpp"
#include "Window.hpp"
#include "Math.hpp"

struct Camera
{
	XMMATRIX view;
	XMMATRIX model;

	XMVECTOR position;
	XMVECTOR forward;
	XMVECTOR up;

	float yaw;
	float pitch;

	float speed;
	bool isMouseCaptured;
};

Camera CreateDefaultCamera()
{
	return Camera{
		XMMatrixIdentity(), //view
		XMMatrixIdentity(), //model

		{0, 2, 3},	//position
		{0, 0, -1}, //forward
		{0, 1, 0},	//up

		0, //yaw
		0, //pitch

		0.1f, //speed
		false //capture mouse
	};
}

inline bool UpdateCamera(InputEvents &events, Camera &camera, Window const &window)
{
	bool isCameraChanged = false;

	//Update relative mouse mode
	if (events.keys[SDL_SCANCODE_F] == eKeyState::Press)
	{
		camera.isMouseCaptured = !camera.isMouseCaptured;
		SDL_ShowCursor(!camera.isMouseCaptured);
	}

	XMVECTOR const worldForward = { 0., 0., -1., 0. };
	XMVECTOR const worldRight = { 1., 0., 0., 0. };

	XMVECTOR forward = {};
	XMVECTOR right = {};
	XMMATRIX const cameraMatrix = math::CreateCameraMatrix(camera.position, camera.yaw, camera.pitch);

	if (IsKeyDown(events, SDL_SCANCODE_W))
	{
		forward = XMVectorScale(XMVector4Transform(worldForward, cameraMatrix), camera.speed);
		isCameraChanged = true;
	}
	if (IsKeyDown(events, SDL_SCANCODE_S))
	{
		forward = XMVectorScale(XMVector4Transform(XMVectorNegate(worldForward), cameraMatrix), camera.speed);
		isCameraChanged = true;
	}
	if (IsKeyDown(events, SDL_SCANCODE_A))
	{
		right = XMVectorScale(XMVector4Transform(XMVectorNegate(worldRight), cameraMatrix), camera.speed);
		isCameraChanged = true;
	}
	if (IsKeyDown(events, SDL_SCANCODE_D))
	{
		right = XMVectorScale(XMVector4Transform(worldRight, cameraMatrix), camera.speed);
		isCameraChanged = true;
	}
	if (IsKeyDown(events, SDL_SCANCODE_Q))
	{
		camera.position.m128_f32[1] += -camera.speed;
		isCameraChanged = true;
	}
	if (IsKeyDown(events, SDL_SCANCODE_E))
	{
		camera.position.m128_f32[1] += camera.speed;
		isCameraChanged = true;
	}

	XMUINT2 const windowSize = GetWindowSize(window);
	if (camera.isMouseCaptured)
	{
		XMFLOAT2 const windowCenter = { windowSize.x / 2.f, windowSize.y / 2.f };
		XMFLOAT2 const mouseDelta = { windowCenter.x - events.mouse.x, windowCenter.y - events.mouse.y };
		SDL_WarpMouseInWindow(window.window, (int)windowCenter.x, (int)windowCenter.y);

		camera.pitch += mouseDelta.y / windowSize.x;
		camera.yaw += mouseDelta.x / windowSize.y;

		isCameraChanged |= (XMVector2Length({ mouseDelta.x, mouseDelta.y }).m128_f32[0]) > 1e-3;
	}

	camera.position = XMVectorAdd(XMVectorAdd(camera.position, forward), right);
	camera.view = math::CreateViewMatrix(camera.position, camera.yaw, camera.pitch);

	return isCameraChanged;
}