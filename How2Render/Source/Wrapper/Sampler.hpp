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

	inline ID3D11SamplerState *CreateSampler(Context const& context, eTextureSamplerFilterType filterType)
	{
		D3D11_SAMPLER_DESC samplerDesc;

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
		samplerDesc.MipLODBias = 0.f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0.f;
		samplerDesc.BorderColor[1] = 0.f;
		samplerDesc.BorderColor[2] = 0.f;
		samplerDesc.BorderColor[3] = 0.f;
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

	inline ID3D11SamplerState *CreateDepthComparisonSampler(Context const& context)
	{
		D3D11_SAMPLER_DESC samplerDesc;

		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.MipLODBias = 0.f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
		samplerDesc.BorderColor[0] = 1.f;
		samplerDesc.BorderColor[1] = 1.f;
		samplerDesc.BorderColor[2] = 1.f;
		samplerDesc.BorderColor[3] = 1.f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ID3D11SamplerState *depthSamplerState;

		auto hr = context.pd3dDevice->CreateSamplerState(&samplerDesc, &depthSamplerState);
		if (FAILED(hr))
		{
			printf("Failed to create depth sampler.");
			return nullptr;
		}

		return depthSamplerState;
	}

	inline void ReleaseSampler(ID3D11SamplerState *sampler)
	{
		if (sampler)
		{
			sampler->Release();
		}
	}

} // namespace h2r
