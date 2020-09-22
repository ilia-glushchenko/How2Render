#pragma once

#include "Wrapper/Context.hpp"
#include <d3d11.h>

namespace h2r
{

	struct TextureSamplers
	{
		ID3D11SamplerState *pPointSampler = nullptr;
		ID3D11SamplerState *pBilinearSampler = nullptr;
		ID3D11SamplerState *pTrilinearSampler = nullptr;
	};

	enum class eTextureSamplerFilterType : uint8_t
	{
		Point = 0,
		Bilinear,
		Trilinear
	};

	inline void BindSampler(
		Context const &context,
		ID3D11SamplerState *sampler)
	{
		context.pImmediateContext->PSSetSamplers(0, 1, &sampler);
	}

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

	inline void CleanupSampler(ID3D11SamplerState *sampler)
	{
		if (sampler)
		{
			sampler->Release();
		}
	}

	inline TextureSamplers CreateTextureSamplers(Context const &context)
	{
		TextureSamplers textureSamplers;

		textureSamplers.pPointSampler = CreateSampler(context, eTextureSamplerFilterType::Point);
		textureSamplers.pBilinearSampler = CreateSampler(context, eTextureSamplerFilterType::Bilinear);
		textureSamplers.pTrilinearSampler = CreateSampler(context, eTextureSamplerFilterType::Trilinear);

		return textureSamplers;
	}

	inline void CleanupTextureSamplers(TextureSamplers &textureSamplers)
	{
		if (textureSamplers.pPointSampler != nullptr)
		{
			textureSamplers.pPointSampler->Release();
			textureSamplers.pPointSampler = nullptr;
		}
		if (textureSamplers.pBilinearSampler != nullptr)
		{
			textureSamplers.pBilinearSampler->Release();
			textureSamplers.pBilinearSampler = nullptr;
		}
		if (textureSamplers.pTrilinearSampler != nullptr)
		{
			textureSamplers.pTrilinearSampler->Release();
			textureSamplers.pTrilinearSampler = nullptr;
		}
	}

} // namespace h2r
