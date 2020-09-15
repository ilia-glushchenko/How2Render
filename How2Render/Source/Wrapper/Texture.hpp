#pragma once

#include "ThirdParty/stb_image.h"
#include "Wrapper/Context.hpp"
#include <d3d11.h>
#include <filesystem>
#include <tuple>
#include <vector>

namespace h2r
{

	struct HostTexture
	{
		struct Descriptor
		{
			std::filesystem::path path;
			uint8_t *pixels = nullptr;
			uint32_t width = 0;
			uint32_t height = 0;
			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		};

		struct MipLevel
		{
			uint32_t width;
			uint32_t height;
			uint32_t lodIndex;
			uint32_t byteSize;
			uint8_t *data;
		};

		std::filesystem::path path;
		DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		//ToDo: Replace with std::vector<uint8_t>
		uint8_t *pixels = nullptr;
		uint32_t width = 0;
		uint32_t height = 0;
		std::vector<MipLevel> mipChain;
	};

	struct DeviceTexture
	{
		std::filesystem::path path;
		ID3D11Texture2D *texture = nullptr;
		ID3D11ShaderResourceView *shaderResourceView = nullptr;
		ID3D11RenderTargetView *renderTargetView = nullptr;
	};

	inline HostTexture CreateHostTexture(HostTexture::Descriptor desc)
	{
		HostTexture hostTexture;

		hostTexture.path = desc.path;
		hostTexture.pixels = desc.pixels;
		hostTexture.width = desc.width;
		hostTexture.height = desc.height;
		hostTexture.format = desc.format;

		uint32_t const imageSize = (uint32_t)(desc.width * desc.height * sizeof(RGBQUAD));
		hostTexture.mipChain.push_back(
			HostTexture::MipLevel{
				hostTexture.width,
				hostTexture.height,
				0U,
				imageSize,
				hostTexture.pixels});

		return hostTexture;
	}

	inline void CleanupHostTexture(HostTexture &hostTexture)
	{
		if (hostTexture.pixels != nullptr)
		{
			stbi_image_free(hostTexture.pixels);
			hostTexture.pixels = nullptr;
		}

		hostTexture.width = hostTexture.height = 0;
		hostTexture.mipChain.clear();
	}

	inline void CleanupDeviceTexture(DeviceTexture &texture)
	{
		if (texture.texture != nullptr)
		{
			texture.texture->Release();
			texture.texture = nullptr;
		}
		if (texture.shaderResourceView != nullptr)
		{
			texture.shaderResourceView->Release();
			texture.shaderResourceView = nullptr;
		}
	}

	inline std::tuple<bool, DeviceTexture> CreateDeviceTexture(Context const &context, HostTexture const &hostTexture)
	{
		DeviceTexture result{hostTexture.path, nullptr, nullptr};

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = hostTexture.width;
		desc.Height = hostTexture.height;
		desc.MipLevels = UINT(hostTexture.mipChain.empty() ? 1 : hostTexture.mipChain.size());
		desc.ArraySize = 1;
		desc.Format = hostTexture.format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		std::vector<D3D11_SUBRESOURCE_DATA> textureData;
		for (const auto &mipLevel : hostTexture.mipChain)
		{
			D3D11_SUBRESOURCE_DATA subData;
			subData.pSysMem = mipLevel.data;
			subData.SysMemPitch = mipLevel.width * sizeof(RGBQUAD);
			subData.SysMemSlicePitch = 0;
			textureData.push_back(subData);
		}

		auto hr = context.pd3dDevice->CreateTexture2D(&desc, textureData.data(), &result.texture);
		if (FAILED(hr))
		{
			printf("Failed to create texture.");
			CleanupDeviceTexture(result);
			return {false, result};
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = desc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = desc.MipLevels;
		if (FAILED(context.pd3dDevice->CreateShaderResourceView(result.texture, &SRVDesc, &result.shaderResourceView)))
		{
			printf("Failed to create shader resource view");
			CleanupDeviceTexture(result);
			return {false, result};
		}

		D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
		RTVDesc.Format = desc.Format;
		RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;
		if (FAILED(context.pd3dDevice->CreateRenderTargetView(result.texture, &RTVDesc, &result.renderTargetView)))
		{
			printf("Failed to create render target view.");
			CleanupDeviceTexture(result);
			return {false, result};
		}

		return {true, result};
	}

	inline std::tuple<bool, DeviceTexture> CreateDeviceTexture(Context const &context, uint32_t width, uint32_t height)
	{
		HostTexture hostTexture;

		hostTexture.width = width;
		hostTexture.height = height;
		hostTexture.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		return CreateDeviceTexture(context, hostTexture);
	}

} // namespace h2r
