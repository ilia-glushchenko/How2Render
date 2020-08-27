#pragma once

#include "Wrapper/Context.hpp"
#include <d3d11.h>

namespace h2r
{

	constexpr uint8_t COLOR_WRITE_ENABLE_RGB = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;

	enum class eBlendStateType : uint8_t
	{
		Additive = 0,
		Normal,
		AlphaToCoverage
	};

	struct BlendStates
	{
		ID3D11BlendState* pAdditiveBlendState = nullptr;
		ID3D11BlendState* pNormalBlendState = nullptr;
		ID3D11BlendState* pAlphaToCoverageBlendState = nullptr;
	};

	inline ID3D11BlendState *CreateBlendState(Context const& context, eBlendStateType blendType,
		uint8_t renderTargetWriteMask = COLOR_WRITE_ENABLE_RGB)
	{
		D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};

		switch (blendType)
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
		rtBlendDesc.RenderTargetWriteMask = renderTargetWriteMask;

		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = (eBlendStateType::AlphaToCoverage == blendType);
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0] = rtBlendDesc;

		ID3D11BlendState *blendState;
		auto hr = context.pd3dDevice->CreateBlendState(&blendDesc, &blendState);
		if (FAILED(hr))
		{
			printf("Failed to create blend state.");
			return nullptr;
		}

		return blendState;
	}

	inline BlendStates CreateBlendStates(Context &context)
	{
		BlendStates blendStates;

		blendStates.pAdditiveBlendState = CreateBlendState(context, eBlendStateType::Additive);
		blendStates.pNormalBlendState = CreateBlendState(context, eBlendStateType::Normal);
		blendStates.pAlphaToCoverageBlendState = CreateBlendState(context, eBlendStateType::AlphaToCoverage);

		return blendStates;
	}

	inline void CleanupBlendStates(BlendStates& blendStates)
	{
		if (blendStates.pAdditiveBlendState)
		{
			blendStates.pAdditiveBlendState->Release();
		}
		if (blendStates.pNormalBlendState)
		{
			blendStates.pNormalBlendState->Release();
		}
		if (blendStates.pAlphaToCoverageBlendState)
		{
			blendStates.pAlphaToCoverageBlendState->Release();
		}
	}

} // namespace h2r
