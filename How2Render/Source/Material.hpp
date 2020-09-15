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

	inline DeviceMaterial CreateDeviceMaterial(Context const &context, TextureCache &cache, HostMaterial const &hostMaterial)
	{
		DeviceMaterial deviceMaterial;

		{
			auto [isTextureCached, cachedTexture] = FindCachedDeviceTexture(cache, hostMaterial.albedoTexture.path);
			if (isTextureCached)
			{
				deviceMaterial.albedoTexture = cachedTexture;
			}
			else if (hostMaterial.albedoTexture.pixels)
			{
				auto [result, texture] = CreateDeviceTexture(context, hostMaterial.albedoTexture);
				if (result)
				{
					deviceMaterial.albedoTexture = texture;
					CacheDeviceTexture(cache, texture);
				}
			}
		}

		{
			auto [isTextureCached, cachedTexture] = FindCachedDeviceTexture(cache, hostMaterial.ambientTexture.path);
			if (isTextureCached)
			{
				deviceMaterial.ambientTexture = cachedTexture;
			}
			else if (hostMaterial.ambientTexture.pixels)
			{
				auto [result, texture] = CreateDeviceTexture(context, hostMaterial.ambientTexture);
				if (result)
				{
					deviceMaterial.ambientTexture = texture;
					CacheDeviceTexture(cache, texture);
				}
			}
		}

		{
			auto [isTextureCached, cachedTexture] = FindCachedDeviceTexture(cache, hostMaterial.specularTexture.path);
			if (isTextureCached)
			{
				deviceMaterial.specularTexture = cachedTexture;
			}
			else if (hostMaterial.specularTexture.pixels)
			{
				auto [result, texture] = CreateDeviceTexture(context, hostMaterial.specularTexture);
				if (result)
				{
					deviceMaterial.specularTexture = texture;
					CacheDeviceTexture(cache, texture);
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
