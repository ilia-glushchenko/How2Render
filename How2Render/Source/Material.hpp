#pragma once

#include "Wrapper/Texture.hpp"
#include "Wrapper/Sampler.hpp"

namespace h2r
{
	constexpr int InvalidMaterialId = -1;

	struct DeviceMaterial
	{
		DeviceTexture ambientTexture;
		DeviceTexture albedoTexture;
		DeviceTexture specularTexture;
        DeviceTexture normalTexture;
		XMFLOAT3 ambient;
		XMFLOAT3 diffuse;
		XMFLOAT3 specular;
		float shininess;
	};
} // namespace h2r
