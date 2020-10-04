#pragma once

#include "Wrapper/BlendState.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/RenderTarget.hpp"
#include "Wrapper/Shader.hpp"

namespace h2r
{

    constexpr uint8_t MAX_RENDER_PASS_RESOURCE_VIEWS_COUNT = 8;
    constexpr uint8_t MAX_RENDER_PASS_RENDER_TARGETS_COUNT = 8;

    enum class ePassType : uint8_t
    {
        None,
        Regular,
        FullScreen,
        Compute,
    };

    struct Pass
    {
        wchar_t const *name = nullptr;
        ePassType type = ePassType::None;
        XMUINT2 viewportSize = {};

        ShaderProgram const *program = nullptr;
        BlendState const *blendState = nullptr;
        ID3D11RasterizerState* rasterizerState = nullptr;
        std::vector<ID3D11SamplerState *> samplerStates = {};
        DeviceConstBuffers const *cbuffers = nullptr;

        std::vector<ID3D11ShaderResourceView *> resourcesVS = {};
        uint32_t resourceOffsetVS = 0;
        std::vector<ID3D11ShaderResourceView *> resourcesPS = {};
        uint32_t resourceOffsetPS = 0;
        std::vector<ID3D11ShaderResourceView *> resourcesCS = {};

        ID3D11DepthStencilState *depthStencilState = nullptr;
        ID3D11DepthStencilView *depthStencilView = nullptr;
        std::vector<ID3D11RenderTargetView *> targets = {};
        std::vector<ID3D11UnorderedAccessView *> targetsCS = {};

        RenderPassClearFlags clearFlags = RENDER_PASS_CLEAR_FLAG_NONE;
        XMVECTORF32 clearValue = DirectX::Colors::Black;
    };

    inline void BindRenderPass(Context const &context, Pass const &pass);

    inline void UnbindRenderPass(Context const &context, Pass const &pass);

} // namespace h2r

namespace h2r
{
    inline void BindShaderResources(
        Context const &context,
        ID3D11ShaderResourceView *const *resourcesVS,
        uint32_t countVS,
        uint32_t offsetVS,
        ID3D11ShaderResourceView *const *resourcesPS,
        uint32_t countPS,
        uint32_t offsetPS,
        ID3D11ShaderResourceView *const *resourcesCS,
        uint32_t countCS)
    {
        if (countVS > 0)
        {
            context.pImmediateContext->VSSetShaderResources(offsetVS, countVS, resourcesVS);
        }
        if (countPS > 0)
        {
            context.pImmediateContext->PSSetShaderResources(offsetPS, countPS, resourcesPS);
        }
        if (countCS > 0)
        {
            context.pImmediateContext->CSSetShaderResources(0, countCS, resourcesCS);
        }
    }

    inline void UnbindShaderResources(
        Context const &context,
        uint32_t countVS,
        uint32_t offsetVS,
        uint32_t countPS,
        uint32_t offsetPS,
        uint32_t countCS)
    {
        static std::vector<ID3D11ShaderResourceView *> nullResources;

        if (countVS > 0)
        {
            nullResources.resize(countVS, nullptr);
            context.pImmediateContext->VSSetShaderResources(offsetVS, countVS, nullResources.data());
        }
        if (countPS > 0)
        {
            nullResources.resize(countPS, nullptr);
            context.pImmediateContext->PSSetShaderResources(offsetPS, countPS, nullResources.data());
        }
        if (countCS > 0)
        {
            nullResources.resize(countCS, nullptr);
            context.pImmediateContext->CSSetShaderResources(0, countCS, nullResources.data());
        }
    }

    inline void BindViewport(Context const &context, uint32_t width, uint32_t height)
    {
        D3D11_VIEWPORT viewport;
        viewport.Width = (FLOAT)width;
        viewport.Height = (FLOAT)height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;

        context.pImmediateContext->RSSetViewports(1, &viewport);
    }

    inline void BindRenderPass(Context const &context, Pass const &pass)
    {
        context.pAnnotation->BeginEvent(pass.name);
        BindShaderResources(
            context,
            pass.resourcesVS.data(),
            (uint32_t)pass.resourcesVS.size(),
            pass.resourceOffsetVS,
            pass.resourcesPS.data(),
            (uint32_t)pass.resourcesPS.size(),
            pass.resourceOffsetPS,
            pass.resourcesCS.data(),
            (uint32_t)pass.resourcesCS.size());
        BindRenderTargets(
            context,
            pass.targets.data(), (uint32_t)pass.targets.size(), pass.depthStencilView,
            pass.targetsCS.data(), (uint32_t)pass.targetsCS.size());

        if (pass.program)
        {
            BindShaders(context, *pass.program);
        }
        if (pass.cbuffers)
        {
            BindConstantBuffers(context, *pass.cbuffers);
        }
        if (!pass.samplerStates.empty())
        {
            BindSamplers(context, pass.samplerStates.data(), (uint32_t)pass.samplerStates.size());
        }

        if (pass.blendState)
        {
            BindBlendState(context, *pass.blendState);
        }
        if (pass.depthStencilView)
        {
            BindDepthStencilState(context, pass.depthStencilState);
        }

        BindViewport(context, pass.viewportSize.x, pass.viewportSize.y);
        if (pass.rasterizerState)
        {
            context.pImmediateContext->RSSetState(pass.rasterizerState);
        }
        ClearRenderTargets(
            context,
            pass.targets.data(), (uint32_t)pass.targets.size(), pass.depthStencilView,
            pass.targetsCS.data(), (uint32_t)pass.targetsCS.size(),
            pass.clearFlags,
            pass.clearValue);
    }

    inline void UnbindRenderPass(Context const &context, Pass const &pass)
    {
        UnbindShaderResources(
            context,
            (uint32_t)pass.resourcesVS.size(),
            pass.resourceOffsetVS,
            (uint32_t)pass.resourcesPS.size(),
            pass.resourceOffsetPS,
            (uint32_t)pass.resourcesCS.size());
        UnbindRenderTargets(context, (uint32_t)pass.targetsCS.size());
        UnbindShaders(context);
        UnbindConstantBuffer(context);
        UnbindSamplers(context, (uint32_t)pass.samplerStates.size());
        UnbindBlendState(context);
        UnbindDepthStencilState(context);
        context.pAnnotation->EndEvent();
    }

} // namespace h2r