#pragma once

#include "ThirdParty/stb_image.h"
#include "Wrapper/Context.hpp"
#include <cstdint>
#include <d3d11.h>
#include <filesystem>
#include <optional>
#include <vector>

namespace h2r
{

	using BindFlags = uint32_t;

	struct HostTexture
	{
		struct Descriptor
		{
			std::filesystem::path path;
			uint8_t *pixels = nullptr;
			uint32_t width = 0;
			uint32_t height = 0;
			DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		};

		struct MipLevel
		{
			uint32_t width;
			uint32_t height;
			uint32_t lodIndex;
			uint32_t byteSize;
			uint32_t byteOffset;
		};

		std::filesystem::path path;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		std::vector<uint8_t> pixels;
		uint32_t width = 0;
		uint32_t height = 0;
		uint8_t texelByteSize = 0;
		std::vector<HostTexture::MipLevel> mipChain;
	};

	struct DeviceTexture
	{
		struct Descriptor
		{
			enum class eMipMapFlag
			{
				USE_PRE_GENERATED,
				USE_DX_GENERATED,
				NONE,
			};

			BindFlags bindFlags = 0;
			eMipMapFlag mipmapFlag = eMipMapFlag::NONE;
			DXGI_FORMAT textureFormat = DXGI_FORMAT_UNKNOWN;
			DXGI_FORMAT srvFormat = DXGI_FORMAT_UNKNOWN;
			DXGI_FORMAT rtvFormat = DXGI_FORMAT_UNKNOWN;
			DXGI_FORMAT uavFormat = DXGI_FORMAT_UNKNOWN;
			DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN;
			HostTexture hostTexture;
		};

		std::filesystem::path path;
		ID3D11Texture2D *texture = nullptr;
		ID3D11ShaderResourceView *shaderResourceView = nullptr;
		ID3D11RenderTargetView *renderTargetView = nullptr;
		ID3D11UnorderedAccessView *unorderedAccessView = nullptr;
		ID3D11DepthStencilView *depthStencilView = nullptr;
		uint32_t width = 0;
		uint32_t height = 0;
	};

	inline HostTexture CreateHostTexture(HostTexture::Descriptor desc);

	inline std::optional<DeviceTexture> CreateDeviceTexture(
		Context const &context, DeviceTexture::Descriptor hostTexture);

	inline void CleanupDeviceTexture(DeviceTexture &texture);

	inline size_t BytesPerPixel(DXGI_FORMAT fmt);

	inline size_t BitsPerPixel(DXGI_FORMAT fmt);

} // namespace h2r

namespace h2r
{

	inline HostTexture CreateHostTexture(HostTexture::Descriptor desc)
	{
		uint32_t const imageSize = (uint32_t)(desc.width * desc.height * BytesPerPixel(desc.format));

		HostTexture hostTexture;

		hostTexture.path = desc.path;
		hostTexture.pixels = std::vector<uint8_t>(desc.pixels, desc.pixels + imageSize);
		hostTexture.width = desc.width;
		hostTexture.height = desc.height;
		hostTexture.format = desc.format;

		hostTexture.mipChain.push_back(HostTexture::MipLevel{hostTexture.width, hostTexture.height, 0U, imageSize, 0});

		return hostTexture;
	}

	inline std::optional<DeviceTexture> CreateDeviceTexture(Context const &context, DeviceTexture::Descriptor desc)
	{
		assert(desc.textureFormat);

		DeviceTexture result;
		result.path = desc.hostTexture.path;
		result.width = desc.hostTexture.width;
		result.height = desc.hostTexture.height;

		D3D11_TEXTURE2D_DESC dxTexDesc;
		dxTexDesc.Width = desc.hostTexture.width;
		dxTexDesc.Height = desc.hostTexture.height;
		dxTexDesc.MipLevels = 1;
		dxTexDesc.ArraySize = 1;
		dxTexDesc.Format = desc.textureFormat;
		dxTexDesc.SampleDesc.Count = 1;
		dxTexDesc.SampleDesc.Quality = 0;
		dxTexDesc.Usage = D3D11_USAGE_DEFAULT;
		dxTexDesc.BindFlags = desc.bindFlags;
		dxTexDesc.CPUAccessFlags = 0;
		dxTexDesc.MiscFlags = 0;

		std::vector<D3D11_SUBRESOURCE_DATA> textureData;
		switch (desc.mipmapFlag)
		{
		case DeviceTexture::Descriptor::eMipMapFlag::USE_PRE_GENERATED:
			dxTexDesc.MipLevels = uint32_t(desc.hostTexture.mipChain.empty() ? 1 : desc.hostTexture.mipChain.size());
			for (const auto &mipLevel : desc.hostTexture.mipChain)
			{
				D3D11_SUBRESOURCE_DATA subData;
				subData.pSysMem = desc.hostTexture.pixels.data() + mipLevel.byteOffset;
				subData.SysMemPitch = mipLevel.width * (UINT)BytesPerPixel(desc.hostTexture.format);
				subData.SysMemSlicePitch = 0;
				textureData.push_back(subData);
			}
			break;
		case DeviceTexture::Descriptor::eMipMapFlag::USE_DX_GENERATED:
			assert(desc.bindFlags & D3D11_BIND_RENDER_TARGET && desc.bindFlags & D3D11_BIND_SHADER_RESOURCE);
			dxTexDesc.MipLevels = 0;
			dxTexDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
			break;
		case DeviceTexture::Descriptor::eMipMapFlag::NONE:
			if (!desc.hostTexture.pixels.empty() && !desc.hostTexture.mipChain.empty())
			{
				auto const &mip = desc.hostTexture.mipChain.front();
				D3D11_SUBRESOURCE_DATA subData;
				subData.pSysMem = desc.hostTexture.pixels.data() + mip.byteOffset;
				subData.SysMemPitch = mip.width * (UINT)BytesPerPixel(desc.hostTexture.format);
				subData.SysMemSlicePitch = 0;
				textureData.push_back(subData);
			}
			break;
		default:
			printf("Invalid MipMap flag");
			assert(true);
			break;
		}

		auto const pSubResourceData = textureData.empty() ? nullptr : textureData.data();
		auto hr = context.pd3dDevice->CreateTexture2D(&dxTexDesc, pSubResourceData, &result.texture);
		if (FAILED(hr))
		{
			printf("Failed to create texture\n");
			CleanupDeviceTexture(result);
			return std::nullopt;
		}

		if (desc.bindFlags & D3D11_BIND_SHADER_RESOURCE)
		{
			assert(desc.srvFormat);

			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.Format = desc.srvFormat;
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = dxTexDesc.MipLevels;

			if (desc.mipmapFlag == DeviceTexture::Descriptor::eMipMapFlag::USE_DX_GENERATED)
			{
				SRVDesc.Texture2D.MostDetailedMip = 0;
				SRVDesc.Texture2D.MipLevels = -1;
				context.pImmediateContext->UpdateSubresource(
					result.texture,
					0,
					nullptr,
					desc.hostTexture.pixels.data(),
					desc.hostTexture.width * (UINT)BytesPerPixel(desc.hostTexture.format),
					0);
			}

			if (FAILED(context.pd3dDevice->CreateShaderResourceView(result.texture, &SRVDesc, &result.shaderResourceView)))
			{
				printf("Failed to create shader resource view\n");
				CleanupDeviceTexture(result);
				return std::nullopt;
			}

			if (desc.mipmapFlag == DeviceTexture::Descriptor::eMipMapFlag::USE_DX_GENERATED)
			{
				context.pImmediateContext->GenerateMips(result.shaderResourceView);
			}
		}

		if (desc.bindFlags & D3D11_BIND_RENDER_TARGET)
		{
			assert(desc.rtvFormat);

			D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
			RTVDesc.Format = desc.rtvFormat;
			RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			RTVDesc.Texture2D.MipSlice = 0;
			if (FAILED(context.pd3dDevice->CreateRenderTargetView(result.texture, &RTVDesc, &result.renderTargetView)))
			{
				printf("Failed to create render target view\n");
				CleanupDeviceTexture(result);
				return std::nullopt;
			}
		}

		if (desc.bindFlags & D3D11_BIND_UNORDERED_ACCESS)
		{
			// No assert here since we can handle DXGI_FORMAT_UNKNOWN

			D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
			ZeroMemory(&descUAV, sizeof(descUAV));
			descUAV.Format = desc.uavFormat;
			descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			descUAV.Texture2D.MipSlice = 0;
			if (FAILED(context.pd3dDevice->CreateUnorderedAccessView(result.texture, &descUAV, &result.unorderedAccessView)))
			{
				printf("Failed to create unordered access view\n");
				CleanupDeviceTexture(result);
				return std::nullopt;
			}
		}

		if (desc.bindFlags & D3D11_BIND_DEPTH_STENCIL)
		{
			assert(desc.dsvFormat);

			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = desc.dsvFormat;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Flags = 0;
			dsvDesc.Texture2D.MipSlice = 0;

			hr = context.pd3dDevice->CreateDepthStencilView(result.texture, &dsvDesc, &result.depthStencilView);
			if (FAILED(hr))
			{
				printf("Failed to create depth/stencil view");
				CleanupDeviceTexture(result);
				return std::nullopt;
			}
		}

		return result;
	}

	inline void CleanupDeviceTexture(DeviceTexture &texture)
	{
		if (texture.shaderResourceView != nullptr)
		{
			texture.shaderResourceView->Release();
			texture.shaderResourceView = nullptr;
		}
		if (texture.renderTargetView != nullptr)
		{
			texture.renderTargetView->Release();
			texture.renderTargetView = nullptr;
		}
		if (texture.unorderedAccessView != nullptr)
		{
			texture.unorderedAccessView->Release();
			texture.unorderedAccessView = nullptr;
		}
		if (texture.depthStencilView != nullptr)
		{
			texture.depthStencilView->Release();
			texture.depthStencilView = nullptr;
		}
		if (texture.texture != nullptr)
		{
			texture.texture->Release();
			texture.texture = nullptr;
		}
	}

	inline size_t BytesPerPixel(DXGI_FORMAT fmt)
	{
		return BitsPerPixel(fmt) / 8;
	}

	inline size_t BitsPerPixel(DXGI_FORMAT fmt)
	{
		switch (static_cast<int>(fmt))
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 128;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 96;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGI_FORMAT_Y416:
		case DXGI_FORMAT_Y210:
		case DXGI_FORMAT_Y216:
			return 64;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_AYUV:
		case DXGI_FORMAT_Y410:
		case DXGI_FORMAT_YUY2:
			return 32;

		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
			return 24;

		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_A8P8:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return 16;

		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_420_OPAQUE:
		case DXGI_FORMAT_NV11:
			return 12;

		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_AI44:
		case DXGI_FORMAT_IA44:
		case DXGI_FORMAT_P8:
			return 8;

		case DXGI_FORMAT_R1_UNORM:
			return 1;

		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			return 4;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return 8;

		default:
			return 0;
		}
	}

} // namespace h2r
