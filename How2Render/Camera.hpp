#pragma once

#include "Input.hpp"
#include "Window.hpp"
#include <DirectXMath.h>

struct Camera
{
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX model;

	DirectX::XMVECTOR position;
	DirectX::XMVECTOR forward;
	DirectX::XMVECTOR up;

	float yaw;
	float pitch;

	float speed;
	bool isMouseCaptured;
};

Camera CreateDefaultCamera()
{
	return Camera{
		DirectX::XMMatrixIdentity(), //view
		DirectX::XMMatrixIdentity(), //model

		{0, 2, 3},	//position
		{0, 0, -1}, //forward
		{0, 1, 0},	//up

		0, //yaw
		0, //pitch

		0.1f, //speed
		false //capture mouse
	};
}

inline DirectX::XMMATRIX CreateCameraMatrix(DirectX::XMVECTOR pos, float yaw, float pitch)
{
	return DirectX::XMMatrixRotationX(pitch) * DirectX::XMMatrixRotationY(yaw) * DirectX::XMMatrixTranslation(pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2]);
}

inline DirectX::XMMATRIX CreateViewMatrix(DirectX::XMVECTOR pos, float yaw, float pitch)
{
	return DirectX::XMMatrixTranslation(-pos.m128_f32[0], -pos.m128_f32[1], -pos.m128_f32[2]) * DirectX::XMMatrixRotationY(-yaw) * DirectX::XMMatrixRotationX(-pitch);
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

	DirectX::XMVECTOR const worldForward = {0.f, 0.f, -1.f, 0.f};
	DirectX::XMVECTOR const worldRight = {1.f, 0.f, 0.f, 0.f};

	DirectX::XMVECTOR forward = {};
	DirectX::XMVECTOR right = {};
	DirectX::XMMATRIX cameraMatrix = CreateCameraMatrix(camera.position, camera.yaw, camera.pitch);

	if (IsKeyDown(events, SDL_SCANCODE_W))
	{
		forward = DirectX::XMVectorScale(DirectX::XMVector4Transform(worldForward, cameraMatrix), camera.speed);
		isCameraChanged = true;
	}
	if (IsKeyDown(events, SDL_SCANCODE_S))
	{
		forward = DirectX::XMVectorScale(
			DirectX::XMVector4Transform(DirectX::XMVectorNegate(worldForward), cameraMatrix),
			camera.speed);
		isCameraChanged = true;
	}
	if (IsKeyDown(events, SDL_SCANCODE_A))
	{
		right = DirectX::XMVectorScale(
			DirectX::XMVector4Transform(DirectX::XMVectorNegate(worldRight), cameraMatrix),
			camera.speed);
		isCameraChanged = true;
	}
	if (IsKeyDown(events, SDL_SCANCODE_D))
	{
		right = DirectX::XMVectorScale(DirectX::XMVector4Transform(worldRight, cameraMatrix), camera.speed);
		isCameraChanged = true;
	}
	if (IsKeyDown(events, SDL_SCANCODE_Q))
	{
		camera.position.m128_f32[1] -= camera.speed;
		isCameraChanged = true;
	}
	if (IsKeyDown(events, SDL_SCANCODE_E))
	{
		camera.position.m128_f32[1] += camera.speed;
		isCameraChanged = true;
	}

	if (camera.isMouseCaptured)
	{
		DirectX::XMUINT2 const windowSize = GetWindowSize(window);
		DirectX::XMUINT2 const windowCenter = {windowSize.x / 2, windowSize.y / 2};
		DirectX::XMFLOAT2 const mouseDistance = {
			static_cast<float>(windowCenter.x - events.mouse.x),
			static_cast<float>(windowCenter.y - events.mouse.y)};
		SDL_WarpMouseInWindow(window.window, windowCenter.x, windowCenter.y);

		camera.pitch += mouseDistance.y / windowSize.x;
		camera.yaw += mouseDistance.x / windowSize.y;

		isCameraChanged |= DirectX::XMVectorGetX(DirectX::XMVector2Length({mouseDistance.x, mouseDistance.y})) > 0;
	}

	camera.position = DirectX::XMVectorAdd(DirectX::XMVectorAdd(camera.position, forward), right);
	camera.view = CreateViewMatrix(camera.position, camera.yaw, camera.pitch);

	return isCameraChanged;
}
