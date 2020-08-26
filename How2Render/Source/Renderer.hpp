#pragma once

#include "Wrapper/ConstantBuffer.hpp"
#include "Helpers/SphereMeshGenerator.hpp"
#include "Helpers/GroundMeshGenerator.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include "ShadowMapPass.hpp"

namespace h2r
{
	void RenderFrame(
		Application const& app,
		Camera const& camera,
		std::initializer_list<RenderObject> const& renderObjects,
		SpotLight const& spotLight)
	{
		ShadowMapPass(app, renderObjects, spotLight);

		// Bind color and depth/stencil render targets
		app.context.pImmediateContext->OMSetRenderTargets(1, &app.swapchain.pRenderTargetView, app.swapchain.pDepthStencilView);
		app.context.pImmediateContext->ClearRenderTargetView(app.swapchain.pRenderTargetView, DirectX::Colors::SkyBlue);
		app.context.pImmediateContext->ClearDepthStencilView(app.swapchain.pDepthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		// Setup vertex shader
		app.context.pImmediateContext->VSSetShader(app.shaders.pVertexShader, nullptr, 0);
		app.context.pImmediateContext->VSSetConstantBuffers(0, 1, &app.shaders.pConstantBuffer);

		// Setup pixel shader
		app.context.pImmediateContext->PSSetShader(app.shaders.pPixelShader, nullptr, 0);
		app.context.pImmediateContext->PSSetConstantBuffers(0, 1, &app.shaders.pConstantBuffer);
		app.context.pImmediateContext->PSSetConstantBuffers(1, 1, &app.shaders.pMaterialConstants);
		app.context.pImmediateContext->PSSetShaderResources(MaxMaterialTextures, 1, &spotLight.shadowMap.texture.pShaderResourceView);
		app.context.pImmediateContext->PSSetSamplers(0, 1, &app.shaders.pTrilinearSampler);
		app.context.pImmediateContext->PSSetSamplers(1, 1, &app.shaders.pDepthComparisonSampler);

		for (auto const& object : renderObjects)
		{
			TransformConstantBuffer transforms;
			XMVECTOR det;

			transforms.world = object.world;
			transforms.worldView = XMMatrixMultiply(object.world, camera.view);
			transforms.worldViewProj = XMMatrixMultiply(object.world, camera.viewProj);
			transforms.normal = XMMatrixTranspose(XMMatrixInverse(&det, transforms.worldView));
			transforms.shadowProj = CalculateShadowProjection(spotLight);
			transforms.lightViewPos = XMVector3Transform(spotLight.position, camera.view);

			// Update transform
			app.context.pImmediateContext->UpdateSubresource(app.shaders.pConstantBuffer, 0, nullptr, &transforms, 0, 0);

			DrawModel(object.model, app.context,  app.shaders, app.blendStates);
		}

		// Cleanup state
		ID3D11ShaderResourceView* const pNullSrv[MaxMaterialTextures + 1] = { nullptr };
		app.context.pImmediateContext->PSSetShaderResources(0, MaxMaterialTextures + 1, pNullSrv);
		app.context.pImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

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
		SpotLight spotLight = CreateDefaultSpotLight(application.context);
		auto ground = GenerateGroundRenderObject(textureLoader);
		auto sphere = GenerateSphereRenderObject(textureLoader);
		auto [result, tree] = CreateRenderObject("Models\\white_oak\\white_oak.obj", textureLoader, 0.1f);
		assert(result);
		// Move stone
		sphere.world = XMMatrixMultiply(sphere.world, XMMatrixTranslation(40.f, 0.f, -8.f));

		while (!inputEvents.quit)
		{
			UpdateInput(inputEvents);
			UpdateCamera(camera, inputEvents, window);

			RenderFrame(
				application,
				camera,
				{ground, sphere, tree},
				spotLight
			);
		}

		FreeRenderObject(ground);
		FreeRenderObject(tree);
		FreeSpotLight(spotLight);
		FreeTextureLoader(textureLoader);
		CleanupApplication(application);
		DestroyWindow(window);
	}

} // namespace h2r
