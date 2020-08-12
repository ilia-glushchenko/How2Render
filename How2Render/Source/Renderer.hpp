#pragma once

#include "Wrapper/ConstantBuffer.hpp"
#include "Helpers/SphereMeshGenerator.hpp"
#include "Camera.hpp"
#include "Application.hpp"
#include "ObjModel.hpp"
#include "Input.hpp"
#include <directxcolors.h>

namespace h2r
{
	void RenderFrame(
		HostConstantBuffer const& constantBuffer,
		Application const& app,
		Camera const& camera,
		ObjModel const& model
	)
	{
		app.context.pImmediateContext->ClearRenderTargetView(app.swapchain.pRenderTargetView, DirectX::Colors::White);
		app.context.pImmediateContext->ClearDepthStencilView(app.swapchain.pDepthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		// Update variables
		app.context.pImmediateContext->UpdateSubresource(app.shaders.pConstantBuffer, 0, nullptr, &constantBuffer, 0, 0);

		// Draw model with multiple meshes and materials
		DrawObjModel(app.context, app.shaders, model);

		ID3D11ShaderResourceView* const pSRV[1] = { nullptr };
		app.context.pImmediateContext->PSSetShaderResources(0, 1, pSRV);

		// Present the information rendered to the back buffer to the front buffer (the screen)
		app.swapchain.pSwapChain->Present(0, 0);
	}

	void MainLoop()
	{
		Window window = CreateNewWindow(1280, 720);
		Camera camera = CreateDefaultCamera();
		InputEvents inputEvents = CreateDefaultInputEvents();
		Application application = CreateApplication(window);
        TextureLoader textureLoader = CreateTextureLoader(application.context, true, true);
		auto [result, objModel] = LoadObjModel("Models\\sponza\\sponza.obj", textureLoader);
		assert(result);

		const XMMATRIX world = XMMatrixScaling(0.1f, 0.1f, 0.1f);
		HostConstantBuffer constBuffer;

		while (!inputEvents.quit)
		{
			UpdateInput(inputEvents);
			UpdateCamera(camera, inputEvents, window);
			constBuffer.world = world;
			constBuffer.worldViewProj = world * camera.viewProj;
			constBuffer.cameraWorldPos = camera.position;

			RenderFrame(
				constBuffer,
				application,
				camera,
				objModel
			);
		}

		FreeObjModel(objModel);
        FreeTextureLoader(textureLoader);
		CleanupApplication(application);
		DestroyWindow(window);
	}

} // namespace h2r
