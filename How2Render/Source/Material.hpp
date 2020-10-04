#pragma once

#include "Helpers/TextureCache.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Sampler.hpp"
#include "Wrapper/Texture.hpp"

namespace h2r
{

	constexpr int32_t InvalidMaterialId = -1;
	constexpr int32_t MaterialTextureCount = 4;

	enum class eAlphaMask : uint8_t
	{
		Opaque,
		Transparent,
	};

	struct HostMaterial
	{
		HostTexture ambientTexture;
		HostTexture albedoTexture;
		HostTexture specularTexture;
		HostTexture normalTexture;
		XMFLOAT3 scalarAmbient = {};
		XMFLOAT3 scalarDiffuse = {};
		XMFLOAT3 scalarSpecular = {};
		float scalarShininess = 0;
		float scalarAlpha = 1.f;
		eAlphaMask alphaMask = eAlphaMask::Opaque;
	};

	struct DeviceMaterial
	{
		DeviceTexture ambientTexture;
		DeviceTexture albedoTexture;
		DeviceTexture specularTexture;
		DeviceTexture normalTexture;
		XMFLOAT3 scalarAmbient = {};
		XMFLOAT3 scalarDiffuse = {};
		XMFLOAT3 scalarSpecular = {};
		float scalarShininess = 0;
		float scalarAlpha = 1.f;
		eAlphaMask alphaMask = eAlphaMask::Opaque;
	};

	inline std::optional<DeviceMaterial> CreateDeviceMaterial(Context const &context, TextureCache &cache, HostMaterial const &hostMaterial)
	{
		DeviceMaterial deviceMaterial;
		DeviceTexture::Descriptor desc;
		desc.bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.mipmapFlag = DeviceTexture::Descriptor::eMipMapFlag::USE_DX_GENERATED;

		{
			auto [isTextureCached, cachedTexture] = FindCachedDeviceTexture(cache, hostMaterial.albedoTexture.path);
			if (isTextureCached)
			{
				deviceMaterial.albedoTexture = cachedTexture;
			}
			else if (!hostMaterial.albedoTexture.pixels.empty())
			{
				desc.hostTexture = hostMaterial.albedoTexture;
				desc.textureFormat = desc.srvFormat = desc.rtvFormat = desc.hostTexture.format;
				auto texture = CreateDeviceTexture(context, desc);
				if (texture)
				{
					deviceMaterial.albedoTexture = texture.value();
					CacheDeviceTexture(cache, texture.value());
				}
			}
		}

		{
			auto [isTextureCached, cachedTexture] = FindCachedDeviceTexture(cache, hostMaterial.ambientTexture.path);
			if (isTextureCached)
			{
				deviceMaterial.ambientTexture = cachedTexture;
			}
			else if (!hostMaterial.ambientTexture.pixels.empty())
			{
				desc.hostTexture = hostMaterial.ambientTexture;
				desc.textureFormat = desc.srvFormat = desc.rtvFormat = desc.hostTexture.format;
				auto texture = CreateDeviceTexture(context, desc);
				if (texture)
				{
					deviceMaterial.ambientTexture = texture.value();
					CacheDeviceTexture(cache, texture.value());
				}
			}
		}

		{
			auto [isTextureCached, cachedTexture] = FindCachedDeviceTexture(cache, hostMaterial.specularTexture.path);
			if (isTextureCached)
			{
				deviceMaterial.specularTexture = cachedTexture;
			}
			else if (!hostMaterial.specularTexture.pixels.empty())
			{
				desc.hostTexture = hostMaterial.specularTexture;
				desc.textureFormat = desc.srvFormat = desc.rtvFormat = desc.hostTexture.format;
				auto texture = CreateDeviceTexture(context, desc);
				if (texture)
				{
					deviceMaterial.specularTexture = texture.value();
					CacheDeviceTexture(cache, texture.value());
				}
			}
		}

		{
			auto [isTextureCached, cachedTexture] = FindCachedDeviceTexture(cache, hostMaterial.normalTexture.path);
			if (isTextureCached)
			{
				deviceMaterial.normalTexture = cachedTexture;
			}
			else if (!hostMaterial.normalTexture.pixels.empty())
			{
				desc.hostTexture = hostMaterial.normalTexture;
				desc.textureFormat = desc.srvFormat = desc.rtvFormat = desc.hostTexture.format;
				auto texture = CreateDeviceTexture(context, desc);
				if (texture)
				{
					deviceMaterial.normalTexture = texture.value();
					CacheDeviceTexture(cache, texture.value());
				}
			}
		}

		deviceMaterial.scalarAmbient = hostMaterial.scalarAmbient;
		deviceMaterial.scalarDiffuse = hostMaterial.scalarDiffuse;
		deviceMaterial.scalarSpecular = hostMaterial.scalarSpecular;
		deviceMaterial.scalarShininess = hostMaterial.scalarShininess;
		deviceMaterial.scalarAlpha = hostMaterial.scalarAlpha;
		deviceMaterial.alphaMask = hostMaterial.alphaMask;

		return deviceMaterial;
	}

} // namespace h2r
