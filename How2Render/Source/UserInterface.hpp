#pragma once

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_impl_dx11.h"
#include "ThirdParty/imgui/imgui_impl_sdl.h"
#include "Window.hpp"
#include "Wrapper/Context.hpp"

namespace h2r
{

	inline void InitUI(Window const &window, Context const &context)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplSDL2_InitForD3D(window.pWindow);
		ImGui_ImplDX11_Init(context.pd3dDevice, context.pImmediateContext);

		// ImGui can draw cursors for us on platforms that don't have those.
		// We don't want that here, since Windows has it's own native cursor.
		auto &io = ImGui::GetIO();
		io.MouseDrawCursor = false;
	}

	inline void BeginDrawUI(Window const &window)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplSDL2_NewFrame(window.pWindow);
		ImGui::NewFrame();
	}

	inline void EndDrawUI()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	inline void CleanupUI()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}

} // namespace h2r