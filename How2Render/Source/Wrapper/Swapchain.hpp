#pragma once

#include "Window.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Swapchain.hpp"
#include <SDL.h>
#include <SDL_syswm.h>
#include <d3d11.h>

struct Swapchain
{
	IDXGISwapChain *pSwapChain = nullptr;
	ID3D11Texture2D *pBackBuffer = nullptr;
	ID3D11RenderTargetView *pRenderTargetView = nullptr;
};

inline Swapchain CreateSwapchain(Window const &window, Context const &context)
{
	Swapchain swapchain;

	auto const windowSize = GetWindowSize(window);
	SDL_SysWMinfo sysWMInfo;
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(window.window, &sysWMInfo);

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	HRESULT hr = S_OK;
	IDXGIFactory *dxgiFactory = nullptr;
	{
		IDXGIDevice *dxgiDevice = nullptr;
		hr = context.pd3dDevice->QueryInterface(__uuidof(IDXGIDevice),
												reinterpret_cast<void **>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter *adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory),
										reinterpret_cast<void **>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	assert(SUCCEEDED(hr));

	// Create swap chain
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = static_cast<uint32_t>(windowSize.x);
	sd.BufferDesc.Height = static_cast<uint32_t>(windowSize.y);
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = sysWMInfo.info.win.window;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	hr = dxgiFactory->CreateSwapChain(context.pd3dDevice, &sd, &swapchain.pSwapChain);
	dxgiFactory->MakeWindowAssociation(sysWMInfo.info.win.window, DXGI_MWA_NO_ALT_ENTER);
	dxgiFactory->Release();
	assert(SUCCEEDED(hr));

	// Create a render target view
	hr = swapchain.pSwapChain->GetBuffer(
		0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&swapchain.pBackBuffer));
	assert(SUCCEEDED(hr));

	hr = context.pd3dDevice->CreateRenderTargetView(
		swapchain.pBackBuffer, nullptr, &swapchain.pRenderTargetView);
	assert(SUCCEEDED(hr));

	context.pImmediateContext->OMSetRenderTargets(1, &swapchain.pRenderTargetView, nullptr);

	// Setup the viewport
	D3D11_VIEWPORT viewport;
	viewport.Width = (FLOAT)windowSize.x;
	viewport.Height = (FLOAT)windowSize.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	context.pImmediateContext->RSSetViewports(1, &viewport);

	return swapchain;
}

void CleanupSwapchain(Swapchain const &swapchain)
{
	if (swapchain.pSwapChain != nullptr)
	{
		swapchain.pSwapChain->Release();
	}
	if (swapchain.pBackBuffer != nullptr)
	{
		swapchain.pBackBuffer->Release();
	}
	if (swapchain.pRenderTargetView != nullptr)
	{
		swapchain.pRenderTargetView->Release();
	}
}
