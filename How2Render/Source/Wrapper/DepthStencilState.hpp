#pragma once

#include "Wrapper/Context.hpp"
#include <d3d11_1.h>

namespace h2r
{

	struct DepthStencilStates
	{
		ID3D11DepthStencilState *pLessReadWrite = nullptr;
		ID3D11DepthStencilState *pEqualRead = nullptr;
		ID3D11DepthStencilState *pDisable = nullptr;
	};

	inline void BindDepthStencilState(Context const &context, ID3D11DepthStencilState *state);

	inline std::optional<DepthStencilStates> CreateDepthStencilStates(Context const &context);

	inline void CleanupDepthStencilStates(DepthStencilStates &states);
} // namespace h2r

namespace h2r
{

	inline void BindDepthStencilState(Context const &context, ID3D11DepthStencilState *state)
	{
		context.pImmediateContext->OMSetDepthStencilState(state, 0);
	}

	inline void UnbindDepthStencilState(Context const& context)
	{
		context.pImmediateContext->OMSetDepthStencilState(nullptr, 0);
	}

	inline std::optional<DepthStencilStates> CreateDepthStencilStates(Context const &context)
	{
		DepthStencilStates states;

		{
			D3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
			if (FAILED(context.pd3dDevice->CreateDepthStencilState(&desc, &states.pLessReadWrite)))
			{
				printf("Failed to create depth stencil state\n");
				return std::nullopt;
			}
		}

		{
			D3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
			desc.DepthFunc = D3D11_COMPARISON_EQUAL;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

			if (FAILED(context.pd3dDevice->CreateDepthStencilState(&desc, &states.pEqualRead)))
			{
				printf("Failed to create depth stencil state\n");
				return std::nullopt;
			}
		}

		{
			D3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
			desc.DepthFunc = D3D11_COMPARISON_EQUAL;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			desc.DepthEnable = FALSE;

			if (FAILED(context.pd3dDevice->CreateDepthStencilState(&desc, &states.pDisable)))
			{
				printf("Failed to create depth stencil state\n");
				return std::nullopt;
			}
		}

		return states;
	}

	inline void CleanupDepthStencilStates(DepthStencilStates &states)
	{
		if (states.pLessReadWrite != nullptr)
		{
			states.pLessReadWrite->Release();
			states.pLessReadWrite = nullptr;
		}
		if (states.pEqualRead != nullptr)
		{
			states.pEqualRead->Release();
			states.pEqualRead = nullptr;
		}
		if (states.pDisable != nullptr)
		{
			states.pDisable->Release();
			states.pDisable = nullptr;
		}
	}

} // namespace h2r