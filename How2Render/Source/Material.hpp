#pragma once

#include "Wrapper/Sampler.hpp"
#include "Wrapper/Texture.hpp"

namespace h2r
{

	struct DeviceMaterial
	{
		DeviceTexture texture;
		ID3D11SamplerState *sampler;
	};

	inline DeviceMaterial CreateMaterial(
		Context const &context, DeviceTexture texture, eTextureSamplerFilterType filter)
	{
		DeviceMaterial material;

		material.texture = texture;
		material.sampler = CreateSampler(context, filter);

		return material;
	}

	inline void ClenupMaterial(DeviceMaterial &material)
	{
		ReleaseTexture(material.texture);
		ReleaseSampler(material.sampler);
	}

} // namespace h2r
