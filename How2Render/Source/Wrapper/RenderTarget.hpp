#pragma once

#include "Swapchain.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Texture.hpp"
#include <DirectXColors.h>
#include <d3d11.h>
#include <optional>

namespace h2r
{

	struct RenderTargetView
	{
		ID3D11Texture2D *renderTargetTexture = nullptr;
		ID3D11RenderTargetView *renderTargetView = nullptr;
		ID3D11ShaderResourceView *renderTargetShaderResourceView = nullptr;

		ID3D11Texture2D *depthStencilTexture = nullptr;
		ID3D11DepthStencilView *depthStencilView = nullptr;
	};

	struct RenderTargetViews
	{
		RenderTargetView shadingPass;
		RenderTargetView translucencyPass;
		RenderTargetView gammaCorrection;
		RenderTargetView presentPass;
	};

	struct RenderTargets
	{
		Swapchain swapchain;
		DeviceTexture basePass;
		DeviceTexture gammaCorrection;
	};

	inline std::optional<RenderTargets> CreateRenderTargets(Context const &context, Window const window, Swapchain swapchain)
	{
		auto const windowSize = GetWindowSize(window);
		RenderTargets renderTargets;
		renderTargets.swapchain = swapchain;

		{
			auto [result, texture] = CreateDeviceTexture(context, windowSize.x, windowSize.y);
			if (!result)
			{
				printf("Failed to create shading pass render target!\n");
				return std::nullopt;
			}

			renderTargets.basePass = texture;
		}

		{
			auto [result, texture] = CreateDeviceTexture(context, windowSize.x, windowSize.y);
			if (!result)
			{
				printf("Failed to create gamma correction render target!\n");
				return std::nullopt;
			}

			renderTargets.gammaCorrection = texture;
		}

		return renderTargets;
	}

	inline void CleanupRenderTargets(RenderTargets &renderTargets)
	{
		CleanupDeviceTexture(renderTargets.basePass);
	}

	inline RenderTargetViews CreateRenderTargetViews(RenderTargets const &targets)
	{
		RenderTargetViews views;

		views.shadingPass.depthStencilTexture = targets.swapchain.depthStencilTexture;
		views.shadingPass.depthStencilView = targets.swapchain.depthStencilView;
		views.shadingPass.renderTargetTexture = targets.basePass.texture;
		views.shadingPass.renderTargetView = targets.basePass.renderTargetView;
		views.shadingPass.renderTargetShaderResourceView = targets.basePass.shaderResourceView;

		views.translucencyPass.depthStencilTexture = targets.swapchain.depthStencilTexture;
		views.translucencyPass.depthStencilView = targets.swapchain.depthStencilView;
		views.translucencyPass.renderTargetTexture = targets.basePass.texture;
		views.translucencyPass.renderTargetView = targets.basePass.renderTargetView;
		views.translucencyPass.renderTargetShaderResourceView = targets.basePass.shaderResourceView;

		views.gammaCorrection.depthStencilTexture = nullptr;
		views.gammaCorrection.depthStencilView = nullptr;
		views.gammaCorrection.renderTargetTexture = targets.gammaCorrection.texture;
		views.gammaCorrection.renderTargetView = targets.gammaCorrection.renderTargetView;
		views.gammaCorrection.renderTargetShaderResourceView = targets.gammaCorrection.shaderResourceView;

		views.presentPass.depthStencilTexture = nullptr;
		views.presentPass.depthStencilView = nullptr;
		views.presentPass.renderTargetTexture = targets.swapchain.renderTargetTexture;
		views.presentPass.renderTargetView = targets.swapchain.renderTargetView;
		views.presentPass.renderTargetShaderResourceView = nullptr;

		return views;
	}

	inline void BindRenderTargetView(Context const &context, RenderTargetView views)
	{
		context.pImmediateContext->OMSetRenderTargets(1, &views.renderTargetView, views.depthStencilView);
	}

	inline void ClearRenderTargetView(Context const &context, RenderTargetView views)
	{
		if (views.renderTargetView)
		{
			context.pImmediateContext->ClearRenderTargetView(
				views.renderTargetView, DirectX::Colors::White);
		}
		if (views.depthStencilView)
		{
			context.pImmediateContext->ClearDepthStencilView(
				views.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
		}
	}

	inline void UnbindRenderTargetView(Context const &context)
	{
		context.pImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
	}

} // namespace h2r