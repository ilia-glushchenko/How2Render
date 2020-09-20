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
		std::vector<ID3D11Texture2D *> renderTargetTextures;
		std::vector<ID3D11RenderTargetView *> renderTargetViews;
		std::vector<ID3D11ShaderResourceView *> shaderResourceViews;

		ID3D11Texture2D *depthStencilTexture = nullptr;
		ID3D11DepthStencilView *depthStencilView = nullptr;
	};

	struct RenderTargetViews
	{
		RenderTargetView forwardShadingPass;
		RenderTargetView gBufferPass;
		RenderTargetView deferredShadingPass;
		RenderTargetView translucencyPass;
		RenderTargetView gammaCorrection;
		RenderTargetView presentPass;
	};

	struct RenderTargets
	{
		struct GBuffers
		{
			DeviceTexture positionTexture;
			DeviceTexture normalTexture;
			DeviceTexture ambientTexture;
			DeviceTexture diffuseTexture;
			DeviceTexture specularTexture;
			DeviceTexture shininessTexture;
		};

		GBuffers gbuffers;
		DeviceTexture basePass;
		DeviceTexture gammaCorrection;
		Swapchain swapchain;
	};

	inline std::optional<RenderTargets::GBuffers> CreateGBuffers(
		Context const &context, uint32_t width, uint32_t height)
	{
		RenderTargets::GBuffers gbuffers;

		{
			auto [result, texture] = CreateDeviceTexture(context, width, height);
			if (!result)
			{
				printf("Failed to create Position GBuffer device texture\n");
				return std::nullopt;
			}
			gbuffers.positionTexture = texture;
		}
		{
			auto [result, texture] = CreateDeviceTexture(context, width, height);
			if (!result)
			{
				printf("Failed to create NormalTe GBuffer device texture\n");
				return std::nullopt;
			}
			gbuffers.normalTexture = texture;
		}
		{
			auto [result, texture] = CreateDeviceTexture(context, width, height);
			if (!result)
			{
				printf("Failed to create AmbientTe GBuffer device texture\n");
				return std::nullopt;
			}
			gbuffers.ambientTexture = texture;
		}
		{
			auto [result, texture] = CreateDeviceTexture(context, width, height);
			if (!result)
			{
				printf("Failed to create DiffuseTe GBuffer device texture\n");
				return std::nullopt;
			}
			gbuffers.diffuseTexture = texture;
		}
		{
			auto [result, texture] = CreateDeviceTexture(context, width, height);
			if (!result)
			{
				printf("Failed to create Specular GBuffer device texture\n");
				return std::nullopt;
			}
			gbuffers.specularTexture = texture;
		}
		{
			auto [result, texture] = CreateDeviceTexture(context, width, height);
			if (!result)
			{
				printf("Failed to create Shininess GBuffer device texture\n");
				return std::nullopt;
			}
			gbuffers.shininessTexture = texture;
		}

		return gbuffers;
	}

	inline std::optional<RenderTargets>
	CreateRenderTargets(Context const &context, Window const window, Swapchain swapchain)
	{
		auto const windowSize = GetWindowSize(window);
		RenderTargets renderTargets;
		renderTargets.swapchain = swapchain;
		renderTargets.gbuffers = CreateGBuffers(context, windowSize.x, windowSize.y).value();

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
		CleanupDeviceTexture(renderTargets.gammaCorrection);
		CleanupDeviceTexture(renderTargets.gbuffers.positionTexture);
		CleanupDeviceTexture(renderTargets.gbuffers.normalTexture);
		CleanupDeviceTexture(renderTargets.gbuffers.ambientTexture);
		CleanupDeviceTexture(renderTargets.gbuffers.diffuseTexture);
		CleanupDeviceTexture(renderTargets.gbuffers.specularTexture);
		CleanupDeviceTexture(renderTargets.gbuffers.shininessTexture);
	}

	inline RenderTargetViews CreateRenderTargetViews(RenderTargets const &targets)
	{
		RenderTargetViews views;

		views.forwardShadingPass.depthStencilTexture = targets.swapchain.depthStencilTexture;
		views.forwardShadingPass.depthStencilView = targets.swapchain.depthStencilView;
		views.forwardShadingPass.renderTargetTextures = {targets.basePass.texture};
		views.forwardShadingPass.renderTargetViews = {targets.basePass.renderTargetView};
		views.forwardShadingPass.shaderResourceViews = {targets.basePass.shaderResourceView};

		{
			views.gBufferPass.depthStencilTexture = targets.swapchain.depthStencilTexture;
			views.gBufferPass.depthStencilView = targets.swapchain.depthStencilView;
			views.gBufferPass.renderTargetTextures = {
				targets.gbuffers.positionTexture.texture,
				targets.gbuffers.normalTexture.texture,
				targets.gbuffers.ambientTexture.texture,
				targets.gbuffers.diffuseTexture.texture,
				targets.gbuffers.specularTexture.texture,
				targets.gbuffers.shininessTexture.texture,
			};
			views.gBufferPass.renderTargetViews = {
				targets.gbuffers.positionTexture.renderTargetView,
				targets.gbuffers.normalTexture.renderTargetView,
				targets.gbuffers.ambientTexture.renderTargetView,
				targets.gbuffers.diffuseTexture.renderTargetView,
				targets.gbuffers.specularTexture.renderTargetView,
				targets.gbuffers.shininessTexture.renderTargetView,
			};
			views.gBufferPass.shaderResourceViews = {
				targets.gbuffers.positionTexture.shaderResourceView,
				targets.gbuffers.normalTexture.shaderResourceView,
				targets.gbuffers.ambientTexture.shaderResourceView,
				targets.gbuffers.diffuseTexture.shaderResourceView,
				targets.gbuffers.specularTexture.shaderResourceView,
				targets.gbuffers.shininessTexture.shaderResourceView,
			};
		}

		views.deferredShadingPass.depthStencilTexture = nullptr;
		views.deferredShadingPass.depthStencilView = nullptr;
		views.deferredShadingPass.renderTargetTextures = {targets.basePass.texture};
		views.deferredShadingPass.renderTargetViews = {targets.basePass.renderTargetView};
		views.deferredShadingPass.shaderResourceViews = {targets.basePass.shaderResourceView};

		views.translucencyPass.depthStencilTexture = targets.swapchain.depthStencilTexture;
		views.translucencyPass.depthStencilView = targets.swapchain.depthStencilView;
		views.translucencyPass.renderTargetTextures = {targets.basePass.texture};
		views.translucencyPass.renderTargetViews = {targets.basePass.renderTargetView};
		views.translucencyPass.shaderResourceViews = {targets.basePass.shaderResourceView};

		views.gammaCorrection.depthStencilTexture = nullptr;
		views.gammaCorrection.depthStencilView = nullptr;
		views.gammaCorrection.renderTargetTextures = {targets.gammaCorrection.texture};
		views.gammaCorrection.renderTargetViews = {targets.gammaCorrection.renderTargetView};
		views.gammaCorrection.shaderResourceViews = {targets.gammaCorrection.shaderResourceView};

		views.presentPass.depthStencilTexture = nullptr;
		views.presentPass.depthStencilView = nullptr;
		views.presentPass.renderTargetTextures = {targets.swapchain.renderTargetTexture};
		views.presentPass.renderTargetViews = {targets.swapchain.renderTargetView};
		views.presentPass.shaderResourceViews = {nullptr};

		return views;
	}

	inline void BindRenderTargetView(Context const &context, RenderTargetView views)
	{
		context.pImmediateContext->OMSetRenderTargets(
			static_cast<uint32_t>(views.renderTargetTextures.size()),
			views.renderTargetViews.data(),
			views.depthStencilView);
	}

	inline void ClearRenderTargetView(Context const &context, RenderTargetView views)
	{
		if (!views.renderTargetViews.empty())
		{
			for (auto view : views.renderTargetViews)
			{
				context.pImmediateContext->ClearRenderTargetView(view, DirectX::Colors::White);
			}
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