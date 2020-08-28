#pragma once

#include "Application.hpp"
#include "Camera.hpp"
#include "Helpers/SphereMeshGenerator.hpp"
#include "Input.hpp"
#include "RenderObject.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include <directxcolors.h>

namespace h2r
{
	void RenderFrame(
		HostConstantBuffer const &constantBuffer,
		Application const &app,
		Camera const &camera,
		RenderObject const &renderObject)
	{
		app.context.pImmediateContext->ClearRenderTargetView(app.swapchain.pRenderTargetView, DirectX::Colors::White);

		// Update variables
		app.context.pImmediateContext->UpdateSubresource(app.shaders.pConstantBuffer, 0, nullptr, &constantBuffer, 0, 0);

		// Setup vertex/pixel shader and bind sampler/texture
		app.context.pImmediateContext->VSSetShader(app.shaders.pVertexShader, nullptr, 0);
		app.context.pImmediateContext->VSSetConstantBuffers(0, 1, &app.shaders.pConstantBuffer);
		app.context.pImmediateContext->PSSetShader(app.shaders.pPixelShader, nullptr, 0);
		app.context.pImmediateContext->PSSetSamplers(0, 1, &renderObject.material.sampler);
		app.context.pImmediateContext->PSSetShaderResources(0, 1, &renderObject.material.texture.shaderResourceView);

		constexpr uint32_t stride = sizeof(Vertex);
		constexpr uint32_t offset = 0;
		DeviceMesh const &sphere = renderObject.mesh;

		// Render sphere
		app.context.pImmediateContext->IASetVertexBuffers(0, 1, &sphere.vertexBuffer.pVertexBuffer, &stride, &offset);
		app.context.pImmediateContext->IASetIndexBuffer(sphere.indexBuffer.pIndexBuffer, sphere.indexBuffer.indexFormat, 0);
		app.context.pImmediateContext->IASetPrimitiveTopology(sphere.topology);
		app.context.pImmediateContext->DrawIndexed(sphere.indexBuffer.indexCount, 0, 0);

		ID3D11ShaderResourceView *const pSRV[1] = {nullptr};
		app.context.pImmediateContext->PSSetShaderResources(0, 1, pSRV);

		// Present the information rendered to the back buffer to the front buffer (the screen)
		app.swapchain.pSwapChain->Present(0, 0);
	}

	void MainLoop()
	{
		Window window = CreateNewWindow(640, 640);
		Camera camera = CreateDefaultCamera();
		InputEvents inputEvents = CreateDefaultInputEvents();
		Application application = CreateApplication(window);
		RenderObject renderObject = GenerateSphereRenderObject(application.context);

		HostConstantBuffer constBuffer;

		while (!inputEvents.quit)
		{
			UpdateInput(inputEvents);
			UpdateCamera(camera, inputEvents, window);
			constBuffer.worldViewProj = renderObject.world * camera.viewProj;

			RenderFrame(
				constBuffer,
				application,
				camera,
				renderObject);
		}

		CleanupRenderObject(renderObject);
		CleanupApplication(application);
		DestroyWindow(window);
	}

} // namespace h2r
