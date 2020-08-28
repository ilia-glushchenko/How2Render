#pragma once

#include "Wrapper/Context.hpp"
#include <d3d11.h>

namespace h2r
{

	enum class eTextureSamplerFilterType : uint8_t
	{
		Point = 0,
		Bilinear,
		Trilinear
	};

	inline ID3D11SamplerState *CreateSampler(Context const &context, eTextureSamplerFilterType filterType)
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		switch (filterType)
		{
		case eTextureSamplerFilterType::Point:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case eTextureSamplerFilterType::Bilinear:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case eTextureSamplerFilterType::Trilinear:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		}
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ID3D11SamplerState *samplerState;
		auto hr = context.pd3dDevice->CreateSamplerState(&samplerDesc, &samplerState);
		if (FAILED(hr))
		{
			printf("Failed to create sampler.");
			return nullptr;
		}

		return samplerState;
	}

	inline void ReleaseSampler(ID3D11SamplerState *sampler)
	{
		if (sampler)
		{
			sampler->Release();
		}
	}

} // namespace h2r
