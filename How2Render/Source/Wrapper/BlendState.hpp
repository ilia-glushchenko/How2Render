#pragma once

#include "Wrapper/Context.hpp"
#include <cstdio>
#include <d3d11.h>
#include <optional>

namespace h2r
{

	constexpr uint8_t COLOR_WRITE_ENABLE_RGB = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;

	enum class eBlendStateType : uint8_t
	{
		Additive = 0,
		Normal,
		AlphaToCoverage
	};

	struct BlendStateDescriptor
	{
		eBlendStateType blendType = eBlendStateType::Additive;
		float *pBlendFactor = nullptr;
		uint32_t sampleMask = 0xFFFFFFFF;
		uint8_t renderTargetWriteMask = COLOR_WRITE_ENABLE_RGB;
	};

	struct BlendState
	{
		ID3D11BlendState *pBlendState = nullptr;
		float *pBlendFactor = nullptr;
		uint32_t sampleMask = 0xFFFFFFFF;
	};

	struct BlendStates
	{
		BlendState additive;
		BlendState normal;
		BlendState alphaToCoverage;
		BlendState none;
	};

	inline void BindBlendState(Context const &context, BlendState const &blendState)
	{
		context.pImmediateContext->OMSetBlendState(
			blendState.pBlendState, blendState.pBlendFactor, blendState.sampleMask);
	}

	inline std::optional<BlendState> CreateBlendState(Context const &context, BlendStateDescriptor desc)
	{
		D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};

		switch (desc.blendType)
		{
		case eBlendStateType::Additive:
			rtBlendDesc.BlendEnable = TRUE;
			rtBlendDesc.SrcBlend = D3D11_BLEND_ONE;
			rtBlendDesc.DestBlend = D3D11_BLEND_ONE;
			rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
			break;
		case eBlendStateType::Normal:
			rtBlendDesc.BlendEnable = TRUE;
			rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
			break;
		case eBlendStateType::AlphaToCoverage:
			rtBlendDesc.BlendEnable = FALSE;
			break;
		}
		rtBlendDesc.SrcBlendAlpha = rtBlendDesc.SrcBlend;
		rtBlendDesc.DestBlendAlpha = rtBlendDesc.DestBlend;
		rtBlendDesc.BlendOpAlpha = rtBlendDesc.BlendOp;
		rtBlendDesc.RenderTargetWriteMask = desc.renderTargetWriteMask;

		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = (eBlendStateType::AlphaToCoverage == desc.blendType);
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0] = rtBlendDesc;

		ID3D11BlendState *blendState = nullptr;
		auto hr = context.pd3dDevice->CreateBlendState(&blendDesc, &blendState);
		if (FAILED(hr))
		{
			printf("Failed to create blend state.");
			return std::nullopt;
		}

		return BlendState{
			blendState,
			desc.pBlendFactor,
			desc.sampleMask};
	}

	inline void CleanupBlendState(BlendState &blendState)
	{
		if (blendState.pBlendState)
		{
			blendState.pBlendState->Release();
			blendState.pBlendState = nullptr;
		}
	}

	inline BlendStates CreateBlendStates(Context const &context)
	{
		BlendStates blendStates;

		BlendStateDescriptor desc;
		desc.pBlendFactor = nullptr;
		desc.sampleMask = 0xFFFFFFFF;
		desc.renderTargetWriteMask = COLOR_WRITE_ENABLE_RGB;

		desc.blendType = eBlendStateType::Additive;
		blendStates.additive = CreateBlendState(context, desc).value();

		desc.blendType = eBlendStateType::Normal;
		blendStates.normal = CreateBlendState(context, desc).value();

		desc.blendType = eBlendStateType::AlphaToCoverage;
		blendStates.alphaToCoverage = CreateBlendState(context, desc).value();

		return blendStates;
	}

	inline void CleanupBlendStates(BlendStates &blendStates)
	{
		CleanupBlendState(blendStates.additive);
		CleanupBlendState(blendStates.normal);
		CleanupBlendState(blendStates.alphaToCoverage);
	}

} // namespace h2r