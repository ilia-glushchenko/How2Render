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
		enum class eShadingType : int32_t
		{
			Forward = 0,
			Deferred = 1,
			Count,
		};

		struct States
		{
			bool showSideBarWindow = true;
			bool drawOpaque = true;
			bool drawTransparent = true;
			bool drawTranslucent = true;
			eShadingType shadingType = eShadingType::Forward;
			double shadingGPUTimeMs = 0;
		};

		Context context;
		Swapchain swapchain;
		Shaders shaders;
		States states;
	};

	inline Application CreateApplication(Window &window)
	{
		Application app;

		app.context = CreateContext();
		app.swapchain = CreateSwapchain(window, app.context);
		app.shaders = CreateShaders(app.context);
		InitUI(window, app.context);

		return app;
	}

	inline void CleanupApplication(Application &application)
	{
		CleanupUI();
		CleanupContext(application.context);
		CleanupSwapchain(application.swapchain);
		CleanupShaders(application.shaders);
	}

} // namespace h2r
