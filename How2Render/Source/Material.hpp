#pragma once

#include "Helpers/TextureCache.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Sampler.hpp"
#include "Wrapper/Texture.hpp"

namespace h2r
{

	constexpr int InvalidMaterialId = -1;

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
				auto texture = CreateDeviceTexture(context, desc);
				if (texture)
				{
					deviceMaterial.specularTexture = texture.value();
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
