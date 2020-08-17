#pragma once

#include "Wrapper/Context.hpp"
#include "Wrapper/Swapchain.hpp"
#include "Wrapper/Shader.hpp"
#include "Wrapper/BlendState.hpp"
#include "Window.hpp"

namespace h2r
{

	struct Application
	{
		Context context;
		Swapchain swapchain;
		Shaders shaders;
		BlendStates blendStates;
	};

	inline Application CreateApplication(Window &window)
	{
		Application app;

		app.context = CreateContext();
		app.swapchain = CreateSwapchain(window, app.context);
		app.shaders = CreateShaders(app.context);
		app.blendStates = CreateBlendStates(app.context);

		return app;
	}

	inline void CleanupApplication(Application &application)
	{
		CleanupContext(application.context);
		CleanupSwapchain(application.swapchain);
		CleanupShaders(application.shaders);
		CleanupBlendStates(application.blendStates);
	}

} // namespace h2r
