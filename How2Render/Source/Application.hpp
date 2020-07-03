#pragma once

#include "Wrapper/Context.hpp"
#include "Wrapper/Swapchain.hpp"
#include "Wrapper/Shader.hpp"
#include "Window.hpp"

struct Application
{
	Context context;
	Swapchain swapchain;
	Shaders shaders;
};

inline Application CreateApplication(Window& window)
{
	Application app;

	app.context = CreateContext();
	app.swapchain = CreateSwapchain(window, app.context);
	app.shaders = CreateShaders(app.context);

	return app;
}

void CleanupApplication(Application &application)
{
	CleanupContext(application.context);
	CleanupSwapchain(application.swapchain);
	CleanupShaders(application.shaders);
}