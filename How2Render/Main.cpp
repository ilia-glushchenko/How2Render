#include "Input.hpp"
#include "Intersection.hpp"
#include "Primitives.hpp"
#include "Window.hpp"
#include <DirectXMath.h>
#include <SDL.h>

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

void WritePixel(Window const &window, DirectX::XMUINT2 pixelCoord, DirectX::XMFLOAT3 color)
{
	DirectX::XMUINT2 const windowSize = GetWindowSize(window);
	uint32_t &pixel = ((uint32_t *)window.surface->pixels)[pixelCoord.x + pixelCoord.y * windowSize.y];

	DirectX::XMUINT3 const pixelColor = {
		static_cast<uint32_t>(color.x * 255.f),
		static_cast<uint32_t>(color.y * 255.f),
		static_cast<uint32_t>(color.z * 255.f)};
	pixel = SDL_MapRGB(window.surface->format, pixelColor.x, pixelColor.y, pixelColor.z);
}

void SamplePixel(Window const &window, DirectX::XMUINT2 pixelCoord, Sphere const &sphere)
{
	DirectX::XMFLOAT3 color = {0, 0, 0};
	Ray const ray = CalculateSampleRay(pixelCoord, GetWindowSize(window));
	if (RaySphereIntersection(sphere, ray))
	{
		color = sphere.albedo;
	}

	WritePixel(window, pixelCoord, color);
}

void MainLoop(Window const &window)
{
	Sphere const sphere = Sphere{
		{0.9f, 0.9f, 0.75f}, // Color
		{0.f, 0.f, -5.f},	 // Position
		0.5f				 // Radius
	};

	InputEvents events = CreateDefaultInputEvents();
	while (!events.quit)
	{
		UpdateInput(events);

		DirectX::XMUINT2 const windowSize = GetWindowSize(window);
		DirectX::XMUINT2 pixelCoord = {0, 0};

		//Let's sample every pixel of the surface
		for (pixelCoord.y = 0; pixelCoord.y < windowSize.y; ++pixelCoord.y)
		{
			for (pixelCoord.x = 0; pixelCoord.x < windowSize.x; ++pixelCoord.x)
			{
				SamplePixel(window, pixelCoord, sphere);
			}
		}

		SDL_UpdateWindowSurface(window.window);
	}
}

int main(int argc, char *args[])
{
	Window window = CreateWindow(640, 640);

	MainLoop(window);

	return 0;
}