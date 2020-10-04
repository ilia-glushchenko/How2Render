#pragma once

#include "Wrapper/Context.hpp"
#include <cstdio>
#include <d3d11_1.h>
#include <optional>

namespace h2r
{

    struct RasterizerStates
    {
        ID3D11RasterizerState *defaultRS = nullptr;
        ID3D11RasterizerState *shadowDepth = nullptr;
    };

    inline std::optional<RasterizerStates> CreateRasterizerStates(Context const &context)
    {
        RasterizerStates result;

        D3D11_RASTERIZER_DESC desc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
        if (FAILED(context.pd3dDevice->CreateRasterizerState(&desc, &result.defaultRS)))
        {
            printf("Failed to create default rasterizer states.\n");
            return std::nullopt;
        }

        desc.DepthBias = 1;
        desc.SlopeScaledDepthBias = 2.f;
        desc.DepthBiasClamp = 0.f;
        if (FAILED(context.pd3dDevice->CreateRasterizerState(&desc, &result.shadowDepth)))
        {
            printf("Failed to create default rasterizer states.\n");
            return std::nullopt;
        }

        return result;
    }

    inline void CleanupRasterizerStates(RasterizerStates &rs)
    {
        if (rs.defaultRS)
        {
            rs.defaultRS->Release();
            rs.defaultRS = nullptr;
        }
        if (rs.shadowDepth)
        {
            rs.shadowDepth->Release();
            rs.shadowDepth = nullptr;
        }
    }

    inline void BindRasterizerState(Context const &context, ID3D11RasterizerState *state)
    {
        context.pImmediateContext->RSSetState(state);
    }

} // namespace h2r
