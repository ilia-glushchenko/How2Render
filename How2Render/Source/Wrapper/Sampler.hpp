#pragma once

#include "Wrapper/Context.hpp"
#include <d3d11.h>
#include <vector>

namespace h2r
{

	struct TextureSamplers
	{
		ID3D11SamplerState *pPointSampler = nullptr;
		ID3D11SamplerState *pBilinearSampler = nullptr;
		ID3D11SamplerState *pTrilinearSampler = nullptr;
		ID3D11SamplerState *pAnisotropicSampler = nullptr;
		ID3D11SamplerState *pDepthComparationSampler = nullptr;
	};

	enum class eTextureSamplerFilterType : uint8_t
	{
		Point = 0,
		Bilinear,
		Trilinear,
		Anisotropic,
	};

	inline void BindSamplers(Context const &context, ID3D11SamplerState *const *samplers, uint32_t samplerCount)
	{
		for (uint32_t i = 0; i < samplerCount; ++i)
		{
			context.pImmediateContext->PSSetSamplers(i, 1, &samplers[i]);
			context.pImmediateContext->CSSetSamplers(i, 1, &samplers[i]);
		}
	}

	inline void UnbindSamplers(Context const &context, uint32_t samplerCount)
	{
		ID3D11SamplerState *nullSampler[1] = {nullptr};
		for (uint32_t i = 0; i < samplerCount; ++i)
		{
			context.pImmediateContext->PSSetSamplers(i, 1, nullSampler);
			context.pImmediateContext->CSSetSamplers(i, 1, nullSampler);
		}
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
		case eTextureSamplerFilterType::Anisotropic:
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.MaxAnisotropy = 16;
			break;
		default:
			printf("Invalid eTextureSamplerFilterType\n");
			assert(true);
			break;
		}
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 1.f;
		samplerDesc.BorderColor[1] = 1.f;
		samplerDesc.BorderColor[2] = 1.f;
		samplerDesc.BorderColor[3] = 1.f;
		samplerDesc.MipLODBias = 0.f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.BorderColor[0] = 0.f;
		samplerDesc.BorderColor[1] = 0.f;
		samplerDesc.BorderColor[2] = 0.f;
		samplerDesc.BorderColor[3] = 0.f;

		ID3D11SamplerState *samplerState;
		auto hr = context.pd3dDevice->CreateSamplerState(&samplerDesc, &samplerState);
		if (FAILED(hr))
		{
			printf("Failed to create sampler\n");
			return nullptr;
		}

		return samplerState;
	}

	inline ID3D11SamplerState *CreateDepthComparisonSampler(Context const &context)
	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(D3D11_DEFAULT);

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

		ID3D11SamplerState *depthSamplerState = nullptr;

		auto hr = context.pd3dDevice->CreateSamplerState(&samplerDesc, &depthSamplerState);
		if (FAILED(hr))
		{
			printf("Failed to create depth sampler.");
			return nullptr;
		}

		return depthSamplerState;
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
		textureSamplers.pAnisotropicSampler = CreateSampler(context, eTextureSamplerFilterType::Anisotropic);
		textureSamplers.pDepthComparationSampler = CreateDepthComparisonSampler(context);

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
		if (textureSamplers.pAnisotropicSampler != nullptr)
		{
			textureSamplers.pAnisotropicSampler->Release();
			textureSamplers.pAnisotropicSampler = nullptr;
		}
		if (textureSamplers.pDepthComparationSampler != nullptr)
		{
			textureSamplers.pDepthComparationSampler->Release();
			textureSamplers.pDepthComparationSampler = nullptr;
		}
	}

} // namespace h2r
