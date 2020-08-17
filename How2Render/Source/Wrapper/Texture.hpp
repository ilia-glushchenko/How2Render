#pragma once

#include "Wrapper/Context.hpp"

#include <d3d11.h>
#include <vector>
#include <tuple>

namespace h2r
{

	struct HostTextureMipLevel
	{
		uint32_t width;
		uint32_t height;
		uint32_t lodIndex;
		uint32_t byteSize;
		uint8_t *data;
	};

	struct HostTexture
	{
		uint8_t *pixels = nullptr;
		uint32_t width = 0;
		uint32_t height = 0;
		std::vector<HostTextureMipLevel> mipChain;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	};

	struct DeviceTexture
	{
		uint32_t width = 0;
		uint32_t height = 0;
		ID3D11Texture2D *pTexture = nullptr;
		ID3D11ShaderResourceView *pShaderResourceView = nullptr;
	};

	inline void ReleaseTexture(DeviceTexture& texture)
	{
		if (texture.pTexture != nullptr)
		{
			texture.pTexture->Release();
			texture.pTexture = nullptr;
		}
		if (texture.pShaderResourceView != nullptr)
		{
			texture.pShaderResourceView->Release();
			texture.pShaderResourceView = nullptr;
		}
	}

	inline std::tuple<bool, DeviceTexture> CreateTexture(Context const& context, HostTexture const& image)
	{
		D3D11_TEXTURE2D_DESC desc;

		desc.Width = image.width;
		desc.Height = image.height;
		desc.MipLevels = (UINT)image.mipChain.size();
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		std::vector<D3D11_SUBRESOURCE_DATA> textureData;
		for (const auto& mipLevel : image.mipChain)
		{
			D3D11_SUBRESOURCE_DATA subData;
			subData.pSysMem = mipLevel.data;
			subData.SysMemPitch = mipLevel.width * sizeof(RGBQUAD);
			subData.SysMemSlicePitch = 0;
			textureData.push_back(subData);
		}

		DeviceTexture result{ 0, 0, nullptr, nullptr };

		auto hr = context.pd3dDevice->CreateTexture2D(&desc, textureData.data(), &result.pTexture);
		if (FAILED(hr))
		{
			printf("Failed to create texture.");
			ReleaseTexture(result);
			return { false, result };
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc ={};
		SRVDesc.Format = desc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = desc.MipLevels;

		if (FAILED(context.pd3dDevice->CreateShaderResourceView(result.pTexture, &SRVDesc, &result.pShaderResourceView)))
		{
			printf("Failed to create shader resource view");
			ReleaseTexture(result);
			return { false, result };
		}

		result.width = image.width;
		result.height = image.height;

		return { true, result };
	}

} // namespace h2r
