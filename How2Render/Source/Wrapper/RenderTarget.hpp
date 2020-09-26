#pragma once

#include "Helpers/TextureGenerator.hpp"
#include "Swapchain.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Texture.hpp"
#include <DirectXColors.h>
#include <d3d11.h>
#include <optional>

namespace h2r
{

    using RenderPassClearFlags = uint8_t;
    constexpr RenderPassClearFlags RENDER_PASS_CLEAR_FLAG_NONE = 0;
    constexpr RenderPassClearFlags RENDER_PASS_CLEAR_FLAG_DEPTH_STENCIL = 1;
    constexpr RenderPassClearFlags RENDER_PASS_CLEAR_FLAG_RENDER_TARGET = 2;
    constexpr RenderPassClearFlags RENDER_PASS_CLEAR_FLAG_FLOAT_UAVS = 4;

    inline std::optional<DeviceTexture> CreateRenderTargetTexture(
        Context const& context,
        uint32_t width, uint32_t height,
        DXGI_FORMAT format,
        uint32_t additionalBindFlags);

    inline void BindRenderTargets(
        Context const& context,
        ID3D11RenderTargetView* const* omViews,
        uint32_t omViewCount,
        ID3D11DepthStencilView* depthStencilView,
        ID3D11UnorderedAccessView* const* csViews,
        uint32_t csViewCount);

    inline void ClearRenderTargets(
        Context const& context,
        ID3D11RenderTargetView* const* omViews,
        uint32_t omViewCount,
        ID3D11DepthStencilView* depthStencilView,
        ID3D11UnorderedAccessView* const* csViews,
        uint32_t csViewCount,
        RenderPassClearFlags clearFlags,
        XMVECTORF32 clearColor);

    inline void UnbindRenderTargets(Context const& context, uint32_t csViewCount);

} // namespace h2r

namespace h2r
{

    inline std::optional<DeviceTexture> CreateRenderTargetTexture(
        Context const &context,
        uint32_t width, uint32_t height,
        DXGI_FORMAT format,
        uint32_t additionalBindFlags)
    {
        HostTexture hostTexture;

        hostTexture.width = width;
        hostTexture.height = height;
        hostTexture.format = format;

        DeviceTexture::Descriptor desc;
        desc.mipmapFlag = DeviceTexture::Descriptor::eMipMapFlag::NONE;
        desc.hostTexture = hostTexture;
        desc.bindFlags |= additionalBindFlags;

        return CreateDeviceTexture(context, desc);
    }

    inline void BindRenderTargets(
        Context const& context,
        ID3D11RenderTargetView* const* omViews,
        uint32_t omViewCount,
        ID3D11DepthStencilView* depthStencilView,
        ID3D11UnorderedAccessView* const* csViews,
        uint32_t csViewCount)
    {
        if (omViewCount > 0)
        {
            context.pImmediateContext->OMSetRenderTargets(omViewCount, omViews, depthStencilView);
        }
        if (csViewCount > 0)
        {
            context.pImmediateContext->CSSetUnorderedAccessViews(0, csViewCount, csViews, nullptr);
        }
    }

    inline void ClearRenderTargets(
        Context const& context,
        ID3D11RenderTargetView* const* omViews,
        uint32_t omViewCount,
        ID3D11DepthStencilView* depthStencilView,
        ID3D11UnorderedAccessView* const* csViews,
        uint32_t csViewCount,
        RenderPassClearFlags clearFlags,
        XMVECTORF32 clearColor)
    {
        if (clearFlags & RENDER_PASS_CLEAR_FLAG_RENDER_TARGET)
        {
            for (uint32_t i = 0; i < omViewCount; ++i)
            {
                context.pImmediateContext->ClearRenderTargetView(omViews[i], clearColor);
            }
        }
        if (clearFlags & RENDER_PASS_CLEAR_FLAG_DEPTH_STENCIL)
        {
            if (depthStencilView)
            {
                context.pImmediateContext->ClearDepthStencilView(
                    depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
            }
        }
        if (clearFlags & RENDER_PASS_CLEAR_FLAG_FLOAT_UAVS)
        {
            for (uint32_t i = 0; i < csViewCount; ++i)
            {
                context.pImmediateContext->ClearUnorderedAccessViewFloat(csViews[i], clearColor);
            }
        }
    }

    inline void UnbindRenderTargets(Context const& context, uint32_t csViewCount)
    {
        context.pImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);

        if (csViewCount > 0)
        {
            std::vector<ID3D11UnorderedAccessView*> nullUAVs(csViewCount, nullptr);
            context.pImmediateContext->CSSetUnorderedAccessViews(0, csViewCount, nullUAVs.data(), nullptr);
        }
    }

} // namespace h2r