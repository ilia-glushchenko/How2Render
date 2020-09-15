#pragma once

#include "Application.hpp"
#include "Camera.hpp"
#include "ForwardShading.hpp"
#include "Helpers/MeshGenerator.hpp"
#include "Helpers/TextureCache.hpp"
#include "Input.hpp"
#include "RenderObject.hpp"
#include "UserInterface.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include "Wrapper/RenderTarget.hpp"
#include "Wrapper/Texture.hpp"
#include <directxcolors.h>

namespace h2r
{

	inline RenderObjectStorage LoadRenderObjectStorage(Context const &context, TextureCache &cache)
	{
		HostModel const sponzaHostModel = LoadObjModel("Data\\Models\\sponza\\sponza.obj", cache).value();
		DeviceModel const sponzaDeviceModel = CreateDeviceModel(context, cache, sponzaHostModel);
		RenderObject const sponzaRenderObject = CreateRenderObject(sponzaDeviceModel, {0, 0, 0}, {0, 0, 0}, 0.1f);

		std::vector<RenderObject> opaqueObjects = {sponzaRenderObject};
		std::vector<RenderObject> translucentObjects = GenerateSpheres(context, cache);

		return RenderObjectStorage{opaqueObjects, translucentObjects};
	}

	inline void CleanupRenderObjectStorage(RenderObjectStorage &storage)
	{
		for (auto &object : storage.opaque)
		{
			CleanupRenderObject(object);
		}
		for (auto &object : storage.translucent)
		{
			CleanupRenderObject(object);
		}
	}

	inline void UpdatePerFrameConstantBuffers(
		Context const &context,
		DeviceConstBuffers const &cbuffers,
		Camera const &camera)
	{
		CameraHostConstBuffer cameraConstants;
		cameraConstants.positionVector = camera.position;
		cameraConstants.projMatrix = camera.proj;
		cameraConstants.viewMatrix = camera.view;

		context.pImmediateContext->UpdateSubresource(cbuffers.pCameraConstants, 0, nullptr, &cameraConstants, 0, 0);
	}

	inline void SortTranslucentRenderObjects(Camera const &camera, std::vector<RenderObject> &translucentObjects)
	{
		std::sort(translucentObjects.begin(), translucentObjects.end(), [&camera](auto const &a, auto const &b) {
			auto const aLength = std::abs(XMVector3Length(a.transform.position - camera.position).m128_f32[0]);
			auto const bLength = std::abs(XMVector3Length(b.transform.position - camera.position).m128_f32[0]);
			return aLength > bLength;
		});
	}

	inline void DrawTransluent(
		Context const &context,
		ForwardShaders const &shaders,
		Application::States const &states,
		std::vector<RenderObject> const &renderObjects)
	{
		if (states.drawTranslucent)
		{
			BindBlendState(context, shaders.blendStates.normal);
			BindShaders(context, shaders.translucentPass);
			BindConstantBuffers(context, shaders.cbuffers);
			BindSamplers(context, shaders.samplers);

			for (auto const &object : renderObjects)
			{
				for (auto const &mesh : object.model.transparentMeshes)
				{
					UpdatePerMeshConstantBuffer(
						context,
						shaders.cbuffers,
						object.model.materials,
						mesh,
						object.transform);

					Draw(context, mesh);
				}
			}
		}
	}

	inline void DrawUI(
		Window const &window,
		Context const &context,
		Application::States &states,
		Camera &camera)
	{
		BeginDrawUI(window);

		if (states.showSideBarWindow)
		{
			ImGui::Begin("Settings", &states.showSideBarWindow);
			ImGui::Text("Camera position");
			ImGui::InputFloat3("Position", camera.position.m128_f32, 2);
			ImGui::Checkbox("Draw opaque", &states.drawOpaque);
			ImGui::Checkbox("Draw transparent", &states.drawTransparent);
			ImGui::Checkbox("Draw translucent", &states.drawTranslucent);
			ImGui::End();
		}

		EndDrawUI();
	}

	inline void Present(
		Context const &context,
		Swapchain const &swapchain,
		RenderTargetView const &sourceView,
		RenderTargetView const &presentView)
	{
		// Unbind shader recourses and present
		ID3D11ShaderResourceView *const pSRV[1] = {nullptr};
		context.pImmediateContext->PSSetShaderResources(0, 1, pSRV);

		// Blit off screen texture to back buffer
		context.pImmediateContext->CopyResource(presentView.renderTargetTexture, sourceView.renderTargetTexture);
		swapchain.pSwapChain->Present(0, 0);
	}

	inline void MainLoop()
	{
		Window window = CreateNewWindow(1200, 720);
		Camera camera = CreateDefaultCamera();
		InputEvents inputs = CreateDefaultInputEvents();
		Application app = CreateApplication(window);
		RenderTargets renderTargets = CreateRenderTargets(app.context, window, app.swapchain).value();

		TextureCache textureCache;
		RenderObjectStorage storage = LoadRenderObjectStorage(app.context, textureCache);
		RenderTargetViews renderTargetViews = CreateRenderTargetViews(renderTargets);

		while (!inputs.quit)
		{
			UpdateInput(inputs);
			UpdateCamera(camera, inputs, window);
			UpdatePerFrameConstantBuffers(app.context, app.forwardShaders.cbuffers, camera);

			BindRenderTargetView(app.context, renderTargetViews.shadingPass);
			ClearRenderTargetView(app.context, renderTargetViews.shadingPass);
			ShadeForward(app.context, app.forwardShaders, app.states, storage.opaque);

			BindRenderTargetView(app.context, renderTargetViews.translucencyPass);
			SortTranslucentRenderObjects(camera, storage.translucent);
			DrawTransluent(app.context, app.forwardShaders, app.states, storage.translucent);

			DrawUI(window, app.context, app.states, camera);

			BindRenderTargetView(app.context, renderTargetViews.presentPass);
			Present(app.context, app.swapchain, renderTargetViews.translucencyPass, renderTargetViews.presentPass);
		}

		CleanupRenderTargets(renderTargets);
		CleanupRenderObjectStorage(storage);
		FlushTextureCache(textureCache);
		CleanupApplication(app);
		CleanupWindow(window);
	}

} // namespace h2r
