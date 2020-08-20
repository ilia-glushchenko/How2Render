#pragma once
#include "Camera.hpp"
#include "Input.hpp"
#include "Intersection.hpp"
#include "Primitives.hpp"
#include "Random.hpp"
#include <SDL.h>
#include <algorithm>
#include <cstdio>

Ray CalculateSampleRay(DirectX::XMUINT2 pixelCoordDC, DirectX::XMUINT2 windowSize)
{
	//We are using pixel coordinates in the device coordinate space, which is defined
	//as non normalized width and height of the screen, where X is in range [0; width),
	//and Y is in range [0; height), ie it's just a pixel position.

	//We than use it to convert to Normalized Device Coordinate space,
	//which is usually defined in the [-1; 1] range for all axes. In the ray tracing pipeline
	//this transform might not be that useful, and to be honest, can be omitted here.
	//But in canonical rasterization pipeline this is the space we get into after
	//the vertex shader and projection matrix transform.
	//(almost, there are some other steps in between, such as clip space which will be covered later)
	DirectX::XMFLOAT2 const pixelCoordNDC = {
		static_cast<float>(pixelCoordDC.x) / (windowSize.x - 1.f) * 2.f - 1.f,
		static_cast<float>(pixelCoordDC.y) / (windowSize.y - 1.f) * 2.f - 1.f,
	};

	//Now we convert NDC coordinates to screen space coordinates, which is yet another coordinates
	//space that we can use sample images. It is two dimensional with [0, 1] X and Y range.
	DirectX::XMFLOAT2 const pixelCoordScreenSpace = {
		(pixelCoordNDC.x + 1.f) * 0.5f,
		(pixelCoordNDC.y + 1.f) * 0.5f,
	};

	//Two of those vectors form 90 degree angle, we can use SS coordinates to linearly interpolate between
	//between those two to calculate prime sample ray direction.
	//This is basically the simplest way possible. We are getting fixed field of view and fixed aspect ratio.
	DirectX::XMVECTOR const topLeftDir = DirectX::XMVector3Normalize({-1, 1, -1});
	DirectX::XMVECTOR const bottomRightDir = DirectX::XMVector3Normalize({1, -1, -1});

	DirectX::XMVECTOR const sampleDirection = DirectX::XMVector3Normalize(
		DirectX::XMVectorLerpV(topLeftDir, bottomRightDir, {pixelCoordScreenSpace.x, pixelCoordScreenSpace.y}));

	return Ray{
		{},
		sampleDirection,
	};
}

void WritePixel(
	Window const &window,
	DirectX::XMUINT2 pixelCoord,
	DirectX::XMFLOAT3 color,
	uint32_t count)
{
	color.x = std::clamp(color.x, 0.f, 1.f);
	color.y = std::clamp(color.y, 0.f, 1.f);
	color.z = std::clamp(color.z, 0.f, 1.f);

	DirectX::XMUINT2 const windowSize = GetWindowSize(window);
	uint32_t &pixel = ((uint32_t *)window.surface->pixels)[pixelCoord.x + pixelCoord.y * windowSize.y];

	uint8_t prevPixelColor[3] = {};
	SDL_GetRGB(pixel, window.surface->format, &prevPixelColor[0], &prevPixelColor[1], &prevPixelColor[2]);
	DirectX::XMFLOAT3 prevColor = {
		std::pow(prevPixelColor[0] / 255.f, 2.2f),
		std::pow(prevPixelColor[1] / 255.f, 2.2f),
		std::pow(prevPixelColor[2] / 255.f, 2.2f),
	};

	auto lerp = [](float a, float b, float t) { return a + t * (b - a); };

	color = {
		std::pow(lerp(prevColor.x, color.x, 1.0f / float(count + 1)), 0.45f),
		std::pow(lerp(prevColor.y, color.y, 1.0f / float(count + 1)), 0.45f),
		std::pow(lerp(prevColor.z, color.z, 1.0f / float(count + 1)), 0.45f),
	};
	DirectX::XMUINT3 const pixelColor = {
		static_cast<uint32_t>(color.x * 255.f),
		static_cast<uint32_t>(color.y * 255.f),
		static_cast<uint32_t>(color.z * 255.f)};
	pixel = SDL_MapRGB(window.surface->format, pixelColor.x, pixelColor.y, pixelColor.z);
}

void SamplePixel(
	Window const &window,
	DirectX::XMUINT2 pixelCoord,
	Scene const &sceneViewSpace,
	uint32_t frameCount)
{
	constexpr uint32_t maxBounceCount = 4;
	DirectX::XMFLOAT3 color = {0, 0, 0};
	DirectX::XMFLOAT3 throughput = {1, 1, 1};

	Ray ray = CalculateSampleRay(pixelCoord, GetWindowSize(window));

	for (uint32_t bounceIndex = 0; bounceIndex < maxBounceCount; ++bounceIndex)
	{
		auto [hit, hitData] = CalculateContact(ray, sceneViewSpace);
		if (!hit)
		{
			break;
		}

		color.x += hitData.material.emissive.x * throughput.x;
		color.y += hitData.material.emissive.y * throughput.y;
		color.z += hitData.material.emissive.z * throughput.z;
		throughput.x *= hitData.material.albedo.x;
		throughput.y *= hitData.material.albedo.y;
		throughput.z *= hitData.material.albedo.z;

		ray = Ray{
			hitData.point,
			DirectX::XMVector3Normalize(
				DirectX::XMVectorAdd(hitData.normal, GenerateNormalDistInsideSphereVector(1.f)))};
	}

	WritePixel(window, pixelCoord, color, frameCount);
}

void RenderFrame(Window const &window, Scene const &scene, uint32_t frameCount)
{
	DirectX::XMUINT2 const windowSize = GetWindowSize(window);

	//Let's sample every pixel of the surface
	DirectX::XMUINT2 pixelCoord = {0, 0};
	for (pixelCoord.y = 0; pixelCoord.y < windowSize.y; ++pixelCoord.y)
	{
		for (pixelCoord.x = 0; pixelCoord.x < windowSize.x; ++pixelCoord.x)
		{
			SamplePixel(window, pixelCoord, scene, frameCount);
		}
	}

	SDL_UpdateWindowSurface(window.window);
}
