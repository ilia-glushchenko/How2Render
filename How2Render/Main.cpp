#include "Wrapper/Texture.hpp"
#include "Wrapper/VertexBuffer.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include "Application.hpp"
#include "Math.hpp"

#include <directxcolors.h>
#include <d3d11.h>

struct RenderTargets
{
	Texture first;
	Texture second;
};

RenderTargets CreateRenderTargets(Context const& context, Window const& window)
{
	auto [aTextureResult, aTexture] = CreateTexture(context, window);
	assert(aTextureResult);

	auto [bTextureResult, bTexture] = CreateTexture(context, window);
	assert(bTextureResult);

	return {aTexture, bTexture};
}

void Render(
	VertexBuffer const& buffer,
	HostConstantBuffer const& constantBuffer, 
	Application const& app,
	RenderTargets const& targets, 
	Camera const& camera, 
	bool clearRenderTarget
)
{
	Texture const& currentFrameTarget = constantBuffer.frameCount % 2 == 1 ? targets.first : targets.second;
	Texture const& prevFrameTarget = constantBuffer.frameCount % 2 == 0 ? targets.first : targets.second;

	// Clear the back buffer
	if (clearRenderTarget) {
		app.context.pImmediateContext->ClearRenderTargetView(
			prevFrameTarget.renderTargetView, DirectX::Colors::Black);
	}
	app.context.pImmediateContext->OMSetRenderTargets(
		1, &currentFrameTarget.renderTargetView, nullptr);
	app.context.pImmediateContext->ClearRenderTargetView(
		currentFrameTarget.renderTargetView, DirectX::Colors::Black);

	// Update variables
	app.context.pImmediateContext->UpdateSubresource(
		app.shaders.pConstantBuffer, 0, nullptr, &constantBuffer, 0, 0);

	// Render a triangle
	app.context.pImmediateContext->VSSetShader(app.shaders.pVertexShader, nullptr, 0);
	app.context.pImmediateContext->PSSetShader(app.shaders.pPixelShader, nullptr, 0);
	app.context.pImmediateContext->PSSetConstantBuffers(0, 1, &app.shaders.pConstantBuffer);
	
	app.context.pImmediateContext->PSSetSamplers(0, 1, &prevFrameTarget.pSamplerLinear);
	app.context.pImmediateContext->PSSetShaderResources(0, 1, &prevFrameTarget.shaderResourceView);
	
	app.context.pImmediateContext->Draw(3, 0);
	ID3D11ShaderResourceView* const pSRV[1] = { nullptr };
	app.context.pImmediateContext->PSSetShaderResources(0, 1, pSRV);
	
	// Present the information rendered to the back buffer to the front buffer (the screen)
	app.context.pImmediateContext->CopyResource(app.swapchain.pBackBuffer, currentFrameTarget.texture);
	app.swapchain.pSwapChain->Present(0, 0);
}

int main(int argc, char *args[])
{
	Window window = CreateNewWindow(640, 640);
	Camera camera = CreateDefaultCamera();
	InputEvents inputEvents = CreateDefaultInputEvents();
	Application application = CreateApplication(window);
	VertexBuffer vertexBuffer = CreateVertexBuffer(application.context);
	RenderTargets renderTargets = CreateRenderTargets(application.context, window);
	
	HostConstantBuffer constBuffer;
	constBuffer.frameCount = 1;
	constBuffer.screenWidth = 640;
	constBuffer.screenHeight = 640;

	while (!inputEvents.quit)
	{
		UpdateInput(inputEvents);
		bool const clearRenderTarget = UpdateCamera(camera, inputEvents, window);
		constBuffer.frameCount = clearRenderTarget ? 1 : constBuffer.frameCount;
		constBuffer.view = XMMatrixTranspose(camera.view);

		Render(
			vertexBuffer, 
			constBuffer, 
			application, 
			renderTargets, 
			camera, 
			clearRenderTarget
		);

		constBuffer.frameCount++;
	}

	DestroyWindow(window);
	return 0;
}