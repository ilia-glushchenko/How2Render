#pragma once

#include "Input.hpp"
#include "Math.hpp"
#include "Window.hpp"

namespace h2r
{

	struct Camera
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
		XMMATRIX viewProj;
		XMMATRIX inverseView;
		XMMATRIX inverseProj;

		XMVECTOR position;
		XMVECTOR forward;
		XMVECTOR up;

		float fov;
		float aspectRatio;
		float zNear;
		float zFar;

		float yaw;
		float pitch;

		float speed;
		bool isMouseCaptured;
	};

	inline Camera CreateDefaultCamera()
	{
		return Camera{
			.world{XMMatrixIdentity()},
			.view{XMMatrixIdentity()},
			.proj{XMMatrixIdentity()},
			.viewProj{XMMatrixIdentity()},
			.inverseView{XMMatrixIdentity()},
			.inverseProj{XMMatrixIdentity()},

			.position{-5.f, 1.5f, 0, 1},
			.forward{0, 0, 1},
			.up{0, 1, 0},

			.fov{XMConvertToRadians(90.f)},
			.aspectRatio{1280.f / 720.f},
			.zNear{0.1f},
			.zFar{30.f},

			.yaw{XMConvertToRadians(90.f)},
			.pitch{0},

			.speed{0.005f},
			.isMouseCaptured{false},
		};
	}

	inline bool UpdateCamera(Camera &camera, InputEvents const &events, Window const &window)
	{
		bool isCameraChanged = false;

		//Update relative mouse mode
		if (events.keys[SDL_SCANCODE_F] == eKeyState::Press)
		{
			camera.isMouseCaptured = !camera.isMouseCaptured;
			SDL_ShowCursor(!camera.isMouseCaptured);
		}

		XMVECTOR const worldForward = {0., 0., 1., 0.};
		XMVECTOR const worldRight = {1., 0., 0., 0.};

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
			camera.position = XMVector4Transform(camera.position, XMMatrixTranslation(0, -camera.speed, 0));
			isCameraChanged = true;
		}
		if (IsKeyDown(events, SDL_SCANCODE_E))
		{
			camera.position = XMVector4Transform(camera.position, XMMatrixTranslation(0, camera.speed, 0));
			isCameraChanged = true;
		}

		XMINT2 const windowSize = GetWindowSize(window);
		if (camera.isMouseCaptured)
		{
			XMFLOAT2 const windowCenter = {windowSize.x / 2.f, windowSize.y / 2.f};
			XMFLOAT2 const mouseDelta = {windowCenter.x - events.mouse.x, windowCenter.y - events.mouse.y};
			SDL_WarpMouseInWindow(window.pWindow, (int)windowCenter.x, (int)windowCenter.y);

			camera.pitch -= (mouseDelta.y / windowSize.x) * 0.1f;
			camera.yaw -= (mouseDelta.x / windowSize.y) * 0.1f;

			isCameraChanged |= (XMVector2Length({mouseDelta.x, mouseDelta.y}).m128_f32[0]) > 1e-3;
		}

		camera.position = XMVectorAdd(XMVectorAdd(camera.position, forward), right);
		camera.view = math::CreateViewMatrix(camera.position, camera.yaw, camera.pitch);
		camera.proj = XMMatrixPerspectiveFovLH(camera.fov, camera.aspectRatio, camera.zNear, camera.zFar);
		camera.viewProj = camera.view * camera.proj;
		camera.inverseView = math::CreateCameraMatrix(camera.position, camera.yaw, camera.pitch);
		XMVECTOR det;
		camera.inverseProj = XMMatrixInverse(&det, camera.proj);

		return isCameraChanged;
	}

} // namespace h2r
