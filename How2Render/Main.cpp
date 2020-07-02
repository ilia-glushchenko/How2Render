#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SIMD_AVX2

#include "Window.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include "Application.hpp"
#include "Texture.hpp"

#include <glm/glm.hpp>
#include <SDL.h>
#include <SDL_syswm.h>

#include <directxcolors.h>
#include <d3d11_4.h>
#include <cstdio>
#include <tuple>

RenderTargets CreateRenderTargets(Application const& app, Window const& window)
{
	auto [aTextureResult, aTexture] = CreateTexture(app, window);
	assert(aTextureResult);

	auto [bTextureResult, bTexture] = CreateTexture(app, window);
	assert(bTextureResult);

	return {aTexture, bTexture};
}

void Render(Application const &app, Camera const& camera, RenderTargets const& targets, ConstantBuffer const& cb, bool clearPrev)
{
	Texture const& currentFrameTarget = cb.frameCount % 2 == 1 ? targets.first : targets.second;
	Texture const& prevFrameTarget = cb.frameCount % 2 == 0 ? targets.first : targets.second;

	// Clear the back buffer
	if (clearPrev) {
		app.pImmediateContext->ClearRenderTargetView(prevFrameTarget.renderTargetView, DirectX::Colors::MidnightBlue);
	}
	app.pImmediateContext->OMSetRenderTargets(1, &currentFrameTarget.renderTargetView, nullptr);
	app.pImmediateContext->ClearRenderTargetView(currentFrameTarget.renderTargetView, DirectX::Colors::MidnightBlue);

	// Update variables
	app.pImmediateContext->UpdateSubresource(app.pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Render a triangle
	app.pImmediateContext->VSSetShader(app.pVertexShader, nullptr, 0);
	app.pImmediateContext->PSSetShader(app.pPixelShader, nullptr, 0);
	app.pImmediateContext->PSSetConstantBuffers(0, 1, &app.pConstantBuffer);
	
	app.pImmediateContext->PSSetSamplers(0, 1, &prevFrameTarget.pSamplerLinear);
	app.pImmediateContext->PSSetShaderResources(0, 1, &prevFrameTarget.shaderResourceView);
	
	app.pImmediateContext->Draw(3, 0);
	ID3D11ShaderResourceView* const pSRV[1] = { nullptr };
	app.pImmediateContext->PSSetShaderResources(0, 1, pSRV);
	
	app.pImmediateContext->CopyResource(app.pBackBuffer, currentFrameTarget.texture);

	// Present the information rendered to the back buffer to the front buffer (the screen)
	app.pSwapChain->Present(0, 0);
}

int main(int argc, char *args[])
{
	Window window = CreateNewWindow(640, 640);
	Camera camera = CreateDefaultCamera();
	ConstantBuffer cb;
	cb.frameCount = 1;
	cb.screenWidth = 640;
	cb.screenHeight = 640;

	auto [appResult, app] = CreateApplication(window);
	if (!appResult)
	{
		printf("Failed to create application");
		return -1;
	}
	auto renderTargets = CreateRenderTargets(app, window);

	InputEvents inputEvents = CreateDefaultInputEvents();
	while (!inputEvents.quit)
	{
		UpdateInput(inputEvents);
		bool clearPrev = UpdateCamera(camera, inputEvents, window);
		cb.frameCount = clearPrev ? 1 : cb.frameCount;
		cb.view = XMMatrixTranspose(DirectX::XMMATRIX(&camera.view[0][0]));

		Render(app, camera, renderTargets, cb, clearPrev);

		cb.frameCount++;
	}

	DestroyWindow(window);
	return 0;
}