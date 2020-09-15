#pragma once

#include "UserInterface.hpp"
#include "Window.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Shader.hpp"
#include "Wrapper/Swapchain.hpp"

namespace h2r
{

	struct Application
	{
		struct States
		{
			bool showSideBarWindow = true;
			bool drawOpaque = true;
			bool drawTransparent = true;
			bool drawTranslucent = true;
		};

		Context context;
		Swapchain swapchain;
		ForwardShaders forwardShaders;
		States states;
	};

	inline Application CreateApplication(Window &window)
	{
		Application app;

		app.context = CreateContext();
		app.swapchain = CreateSwapchain(window, app.context);
		app.forwardShaders = CreateForwardShaders(app.context);
		InitUI(window, app.context);

		return app;
	}

	inline void CleanupApplication(Application &application)
	{
		CleanupUI();
		CleanupContext(application.context);
		CleanupSwapchain(application.swapchain);
		CleanupForwardShaders(application.forwardShaders);
	}

} // namespace h2r
