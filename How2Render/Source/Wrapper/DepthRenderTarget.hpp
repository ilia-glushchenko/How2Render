#pragma once

#include "Context.hpp"
#include "Texture.hpp"

namespace h2r
{
	enum class eDepthPrecision : uint8_t
	{
		Unorm16 = 0,
		Unorm24,
		Float32
	};

	struct DepthRenderTarget
	{
		DeviceTexture texture;
		ID3D11DepthStencilView *pDepthStencilView = nullptr;
	};

	std::tuple<bool, DepthRenderTarget> CreateDepthRenderTarget(Context const& context, uint32_t width, uint32_t height, eDepthPrecision precision)
	{
		DXGI_FORMAT texFormat, srvFormat, dsvFormat;

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

		D3D11_TEXTURE2D_DESC desc;

		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = texFormat;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		DepthRenderTarget result{ 0, 0, nullptr, nullptr, nullptr };

		auto hr = context.pd3dDevice->CreateTexture2D(&desc, nullptr, &result.texture.pTexture);
		if (FAILED(hr))
		{
			printf("Failed to create depth texture.");
			ReleaseTexture(result.texture);
			return { false, result };
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc ={};
		srvDesc.Format = srvFormat;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;

		hr = context.pd3dDevice->CreateShaderResourceView(result.texture.pTexture, &srvDesc, &result.texture.pShaderResourceView);
		if (FAILED(hr))
		{
			printf("Failed to create shader resource view");
			ReleaseTexture(result.texture);
			return { false, result };
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = dsvFormat;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
		dsvDesc.Texture2D.MipSlice = 0;

		hr = context.pd3dDevice->CreateDepthStencilView(result.texture.pTexture, &dsvDesc, &result.pDepthStencilView);
		if (FAILED(hr))
		{
			printf("Failed to create depth/stencil view");
			ReleaseTexture(result.texture);
			return { false, result };
		}

		result.texture.width = width;
		result.texture.height = height;

		return { true, result };
	}

	void ReleaseDepthRenderTarget(DepthRenderTarget& depthRenderTarget)
	{
		ReleaseTexture(depthRenderTarget.texture);

		if (depthRenderTarget.pDepthStencilView != nullptr)
		{
			depthRenderTarget.pDepthStencilView->Release();
			depthRenderTarget.pDepthStencilView = nullptr;
		}
	}

} // namespace h2r
