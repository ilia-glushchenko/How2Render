#pragma once

#include "Window.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Swapchain.hpp"
#include <SDL.h>
#include <SDL_syswm.h>
#include <d3d11.h>

namespace h2r
{

	struct Swapchain
	{
		IDXGISwapChain *pSwapChain = nullptr;
		ID3D11Texture2D *renderTargetTexture = nullptr;
		ID3D11RenderTargetView *renderTargetView = nullptr;
		ID3D11Texture2D *depthStencilTexture = nullptr;
		ID3D11DepthStencilView *depthStencilView = nullptr;
		ID3D11ShaderResourceView *depthStencilShaderResourceView = nullptr;
		uint32_t width = 0;
		uint32_t height = 0;
	};

	inline Swapchain CreateSwapchain(Window const &window, Context const &context)
	{
		Swapchain swapchain = {};
		auto const windowSize = GetWindowSize(window);
		swapchain.width = windowSize.x;
		swapchain.height = windowSize.y;

		// Create swapchain
		{
			SDL_SysWMinfo sysWMInfo;
			SDL_VERSION(&sysWMInfo.version);
			SDL_GetWindowWMInfo(window.pWindow, &sysWMInfo);

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
		}

		// Create RenderTarget view
		{
			auto hr = swapchain.pSwapChain->GetBuffer(
				0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&swapchain.renderTargetTexture));
			assert(SUCCEEDED(hr));

			hr = context.pd3dDevice->CreateRenderTargetView(
				swapchain.renderTargetTexture, nullptr, &swapchain.renderTargetView);
			assert(SUCCEEDED(hr));
		}

		// Setup depth/stencil buffer
		{
			D3D11_TEXTURE2D_DESC depthDesc;
			depthDesc.Width = static_cast<uint32_t>(windowSize.x);
			depthDesc.Height = static_cast<uint32_t>(windowSize.y);
			depthDesc.MipLevels = 1;
			depthDesc.ArraySize = 1;
			depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			depthDesc.SampleDesc.Count = 1;
			depthDesc.SampleDesc.Quality = 0;
			depthDesc.Usage = D3D11_USAGE_DEFAULT;
			depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			depthDesc.CPUAccessFlags = 0;
			depthDesc.MiscFlags = 0;

			auto hr = context.pd3dDevice->CreateTexture2D(&depthDesc, nullptr, &swapchain.depthStencilTexture);
			assert(SUCCEEDED(hr));

			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = 0;
			dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;
			hr = context.pd3dDevice->CreateDepthStencilView(
				swapchain.depthStencilTexture, &dsvDesc, &swapchain.depthStencilView);
			assert(SUCCEEDED(hr));

			D3D11_SHADER_RESOURCE_VIEW_DESC dsrvDesc;
			dsrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			dsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			dsrvDesc.Texture2D.MostDetailedMip = 0;
			dsrvDesc.Texture2D.MipLevels = -1;
			hr = context.pd3dDevice->CreateShaderResourceView(
				swapchain.depthStencilTexture, &dsrvDesc, &swapchain.depthStencilShaderResourceView);
			assert(SUCCEEDED(hr));
		}

		// Setup the viewport
		{
			D3D11_VIEWPORT viewport;
			viewport.Width = (FLOAT)windowSize.x;
			viewport.Height = (FLOAT)windowSize.y;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			context.pImmediateContext->RSSetViewports(1, &viewport);
		}

		return swapchain;
	}

	inline void CleanupSwapchain(Swapchain &swapchain)
	{
		if (swapchain.renderTargetTexture != nullptr)
		{
			swapchain.renderTargetTexture->Release();
			swapchain.renderTargetTexture = nullptr;
		}
		if (swapchain.depthStencilTexture != nullptr)
		{
			swapchain.depthStencilTexture->Release();
			swapchain.depthStencilTexture = nullptr;
		}
		if (swapchain.renderTargetView != nullptr)
		{
			swapchain.renderTargetView->Release();
			swapchain.renderTargetView = nullptr;
		}
		if (swapchain.depthStencilView != nullptr)
		{
			swapchain.depthStencilView->Release();
			swapchain.depthStencilView = nullptr;
		}
		if (swapchain.pSwapChain != nullptr)
		{
			swapchain.pSwapChain->Release();
			swapchain.pSwapChain = nullptr;
		}
	}

} // namespace h2r
