#pragma once

#include "Helpers/TextureGenerator.hpp"
#include "Swapchain.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Texture.hpp"
#include <DirectXColors.h>
#include <d3d11.h>
#include <optional>

namespace h2r
{

	using RenderPassClearFlags = uint8_t;
	constexpr RenderPassClearFlags RENDER_PASS_CLEAR_FLAG_NONE = 0;
	constexpr RenderPassClearFlags RENDER_PASS_CLEAR_FLAG_DEPTH_STENCIL = 1;
	constexpr RenderPassClearFlags RENDER_PASS_CLEAR_FLAG_RENDER_TARGET = 2;
	constexpr RenderPassClearFlags RENDER_PASS_CLEAR_FLAG_FLOAT_UAVS = 4;

	enum class eDepthPrecision : uint8_t
	{
		Unorm16 = 0,
		Unorm24,
		Float32
	};

	inline std::optional<DeviceTexture> CreateRenderTargetTexture(
		Context const &context,
		uint32_t width, uint32_t height,
		DXGI_FORMAT format,
		uint32_t additionalBindFlags);

	inline void BindRenderTargets(
		Context const &context,
		ID3D11RenderTargetView *const *omViews,
		uint32_t omViewCount,
		ID3D11DepthStencilView *depthStencilView,
		ID3D11UnorderedAccessView *const *csViews,
		uint32_t csViewCount);

	inline void ClearRenderTargets(
		Context const &context,
		ID3D11RenderTargetView *const *omViews,
		uint32_t omViewCount,
		ID3D11DepthStencilView *depthStencilView,
		ID3D11UnorderedAccessView *const *csViews,
		uint32_t csViewCount,
		RenderPassClearFlags clearFlags,
		XMVECTORF32 clearColor);

	inline void UnbindRenderTargets(Context const &context, uint32_t csViewCount);

} // namespace h2r

namespace h2r
{

	inline void BindRenderTargets(
		Context const &context,
		ID3D11RenderTargetView *const *omViews,
		uint32_t omViewCount,
		ID3D11DepthStencilView *depthStencilView,
		ID3D11UnorderedAccessView *const *csViews,
		uint32_t csViewCount)
	{
		if (omViewCount > 0 || depthStencilView)
		{
			context.pImmediateContext->OMSetRenderTargets(omViewCount, omViews, depthStencilView);
		}
		if (csViewCount > 0)
		{
			context.pImmediateContext->CSSetUnorderedAccessViews(0, csViewCount, csViews, nullptr);
		}
	}

	inline void ClearRenderTargets(
		Context const &context,
		ID3D11RenderTargetView *const *omViews,
		uint32_t omViewCount,
		ID3D11DepthStencilView *depthStencilView,
		ID3D11UnorderedAccessView *const *csViews,
		uint32_t csViewCount,
		RenderPassClearFlags clearFlags,
		XMVECTORF32 clearColor)
	{
		if (clearFlags & RENDER_PASS_CLEAR_FLAG_RENDER_TARGET)
		{
			for (uint32_t i = 0; i < omViewCount; ++i)
			{
				context.pImmediateContext->ClearRenderTargetView(omViews[i], clearColor);
			}
		}
		if (clearFlags & RENDER_PASS_CLEAR_FLAG_DEPTH_STENCIL)
		{
			if (depthStencilView)
			{
				context.pImmediateContext->ClearDepthStencilView(
					depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
			}
		}
		if (clearFlags & RENDER_PASS_CLEAR_FLAG_FLOAT_UAVS)
		{
			for (uint32_t i = 0; i < csViewCount; ++i)
			{
				context.pImmediateContext->ClearUnorderedAccessViewFloat(csViews[i], clearColor);
			}
		}
	}

	inline void UnbindRenderTargets(Context const &context, uint32_t csViewCount)
	{
		context.pImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);

		if (csViewCount > 0)
		{
			std::vector<ID3D11UnorderedAccessView *> nullUAVs(csViewCount, nullptr);
			context.pImmediateContext->CSSetUnorderedAccessViews(0, csViewCount, nullUAVs.data(), nullptr);
		}
	}

	inline std::optional<DeviceTexture> CreateRenderTargetTexture(
		Context const &context,
		uint32_t width, uint32_t height,
		DXGI_FORMAT format)
	{
		HostTexture hostTexture;

		hostTexture.width = width;
		hostTexture.height = height;
		hostTexture.format = format;

		DeviceTexture::Descriptor desc;
		desc.bindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.mipmapFlag = DeviceTexture::Descriptor::eMipMapFlag::NONE;
		desc.textureFormat = hostTexture.format;
		desc.srvFormat = hostTexture.format;
		desc.rtvFormat = hostTexture.format;
		desc.uavFormat = DXGI_FORMAT_UNKNOWN;
		desc.hostTexture = hostTexture;

		return CreateDeviceTexture(context, desc);
	}

	inline std::optional<DeviceTexture> CreateComputeTargetTexture(
		Context const &context,
		uint32_t width, uint32_t height,
		DXGI_FORMAT format)
	{
		HostTexture hostTexture;

		hostTexture.width = width;
		hostTexture.height = height;
		hostTexture.format = format;

		DeviceTexture::Descriptor desc;
		desc.bindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.mipmapFlag = DeviceTexture::Descriptor::eMipMapFlag::NONE;
		desc.textureFormat = hostTexture.format;
		desc.srvFormat = hostTexture.format;
		desc.rtvFormat = hostTexture.format;
		desc.uavFormat = DXGI_FORMAT_UNKNOWN;
		desc.hostTexture = hostTexture;

		return CreateDeviceTexture(context, desc);
	}

	inline std::optional<DeviceTexture> CreateDepthStencilTexture(
		Context const &context, uint32_t width, uint32_t height, eDepthPrecision precision)
	{
		DXGI_FORMAT texFormat, srvFormat, dsvFormat;
		texFormat = srvFormat = dsvFormat = DXGI_FORMAT_UNKNOWN;

		switch (precision)
		{
		case eDepthPrecision::Unorm16:
			texFormat = DXGI_FORMAT_R16_TYPELESS;
			srvFormat = DXGI_FORMAT_R16_UNORM;
			dsvFormat = DXGI_FORMAT_D16_UNORM;
			break;
		case eDepthPrecision::Unorm24:
			texFormat = DXGI_FORMAT_R24G8_TYPELESS;
			srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		case eDepthPrecision::Float32:
			texFormat = DXGI_FORMAT_R32_TYPELESS;
			srvFormat = DXGI_FORMAT_R32_FLOAT;
			dsvFormat = DXGI_FORMAT_D32_FLOAT;
			break;
		default:
			assert(0);
		}

		HostTexture hostTexture;

		hostTexture.width = width;
		hostTexture.height = height;
		hostTexture.format = texFormat;

		DeviceTexture::Descriptor desc;
		desc.bindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		desc.mipmapFlag = DeviceTexture::Descriptor::eMipMapFlag::NONE;
		desc.textureFormat = texFormat;
		desc.srvFormat = srvFormat;
		desc.dsvFormat = dsvFormat;
		desc.hostTexture = hostTexture;

		return CreateDeviceTexture(context, desc);
	}

} // namespace h2r