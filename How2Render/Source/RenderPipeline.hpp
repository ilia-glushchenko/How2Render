#pragma once

#include "Application.hpp"
#include "RenderPass.hpp"
#include <cstdint>

namespace h2r
{

    struct Pipeline
    {
        struct Textures
        {
            struct GBuffers
            {
                DeviceTexture normalXY;
                DeviceTexture ambientRG;
                DeviceTexture diffuseAmbientB;
                DeviceTexture specularShininess;
            };

            DeviceTexture ao1;
            DeviceTexture ao2;
            GBuffers gbuffers;
            DeviceTexture basePass;
            DeviceTexture gammaCorrection;
            DeviceTexture debug;
            Swapchain swapchain;

            DeviceTexture noiseTexture;
        };

        Pass depthPrePassOpaque;
        Pass depthPrePassTransparent;
        Pass deferredGBufferPassOpaque;
        Pass ssao;
        Pass ssaoVerticalBlurPass;
        Pass ssaoHorizontalBlurPass;
        Pass deferredShadingOpaque;
        Pass forwardShadingOpaque;
        Pass forwardShadingTransparent;
        Pass forwardShadingTranclucent;
        Pass gammaCorrection;
        Pass debug;
        Pass ui;
    };

    inline std::optional<Pipeline::Textures> CreatePipelineTextures(Context const &context, Swapchain const &swapchain);

    inline void CleanupPipelineTextures(Pipeline::Textures &textures);

    inline Pipeline CreatePipeline(Application const &app, Pipeline::Textures const &targets);

} // namespace h2r

namespace h2r
{

    inline std::optional<Pipeline::Textures::GBuffers> CreateGBuffers(
        Context const &context, uint32_t width, uint32_t height)
    {
        Pipeline::Textures::GBuffers gbuffers;

        {
            auto texture = CreateRenderTargetTexture(context, width, height, DXGI_FORMAT_R8G8_UNORM, 0);
            if (!texture)
            {
                printf("Failed to create Normal GBuffer device texture\n");
                return std::nullopt;
            }
            gbuffers.normalXY = texture.value();
        }
        {
            auto texture = CreateRenderTargetTexture(context, width, height, DXGI_FORMAT_R8G8_UNORM, 0);
            if (!texture)
            {
                printf("Failed to create Ambient GBuffer device texture\n");
                return std::nullopt;
            }
            gbuffers.ambientRG = texture.value();
        }
        {
            auto texture = CreateRenderTargetTexture(context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
            if (!texture)
            {
                printf("Failed to create Diffuse GBuffer device texture\n");
                return std::nullopt;
            }
            gbuffers.diffuseAmbientB = texture.value();
        }
        {
            auto texture = CreateRenderTargetTexture(context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
            if (!texture)
            {
                printf("Failed to create Specular GBuffer device texture\n");
                return std::nullopt;
            }
            gbuffers.specularShininess = texture.value();
        }

        return gbuffers;
    }

    inline std::optional<Pipeline::Textures>
    CreatePipelineTextures(Context const &context, Swapchain const &swapchain)
    {
        auto const windowSize = XMUINT2(swapchain.width, swapchain.height);
        Pipeline::Textures textures;
        textures.swapchain = swapchain;
        textures.gbuffers = CreateGBuffers(context, windowSize.x, windowSize.y).value();

        {
            auto texture = CreateRenderTargetTexture(
                context, windowSize.x, windowSize.y, DXGI_FORMAT_R8_UNORM, D3D11_BIND_UNORDERED_ACCESS);
            if (!texture)
            {
                printf("Failed to create ao render target!\n");
                return std::nullopt;
            }
            textures.ao1 = texture.value();
        }

        {
            auto texture = CreateRenderTargetTexture(
                context, windowSize.x, windowSize.y, DXGI_FORMAT_R8_UNORM, D3D11_BIND_UNORDERED_ACCESS);
            if (!texture)
            {
                printf("Failed to create ao render target!\n");
                return std::nullopt;
            }
            textures.ao2 = texture.value();
        }

        {
            auto texture = CreateRenderTargetTexture(context, windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
            if (!texture)
            {
                printf("Failed to create shading pass render target!\n");
                return std::nullopt;
            }
            textures.basePass = texture.value();
        }

        {
            auto texture = CreateRenderTargetTexture(context, windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
            if (!texture)
            {
                printf("Failed to create gamma correction render target!\n");
                return std::nullopt;
            }
            textures.gammaCorrection = texture.value();
        }

        {
            auto texture = CreateRenderTargetTexture(context, windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
            if (!texture)
            {
                printf("Failed to create debug render target!\n");
                return std::nullopt;
            }
            textures.debug = texture.value();
        }

        {
            textures.noiseTexture = GenerateNoiseTexture(context, 4, 4);
        }

        return textures;
    }

    inline void CleanupPipelineTextures(Pipeline::Textures &textures)
    {
        CleanupDeviceTexture(textures.ao1);
        CleanupDeviceTexture(textures.ao2);
        CleanupDeviceTexture(textures.basePass);
        CleanupDeviceTexture(textures.gammaCorrection);
        CleanupDeviceTexture(textures.gbuffers.normalXY);
        CleanupDeviceTexture(textures.gbuffers.ambientRG);
        CleanupDeviceTexture(textures.gbuffers.diffuseAmbientB);
        CleanupDeviceTexture(textures.gbuffers.specularShininess);
        CleanupDeviceTexture(textures.noiseTexture);
        CleanupDeviceTexture(textures.debug);
    }

    inline Pipeline CreatePipeline(Application const &app, Pipeline::Textures const &targets)
    {
        Pipeline pipeline = Pipeline{
            .depthPrePassOpaque{
                .name{L"Depth pre-pass opaque"},
                .type{ePassType::Regular},
                .program{&app.shaders.depthPrePassOpaque},
                .blendState{&app.shaders.blendStates.none},
                .samplerStates{app.shaders.samplers.pTrilinearSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{app.shaders.depthStencilStates.pLessReadWrite},
                .depthStencilView{app.swapchain.depthStencilView},
                .targets{targets.gbuffers.normalXY.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_DEPTH_STENCIL | RENDER_PASS_CLEAR_FLAG_RENDER_TARGET},
                .clearValue{DirectX::Colors::Black},
            },
            .depthPrePassTransparent{
                .name{L"Depth pre-pass transparent"},
                .type{ePassType::Regular},
                .program{&app.shaders.depthPrePassTransparent},
                .blendState{&app.shaders.blendStates.none},
                .samplerStates{app.shaders.samplers.pTrilinearSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{app.shaders.depthStencilStates.pLessReadWrite},
                .depthStencilView{app.swapchain.depthStencilView},
                .targets{targets.gbuffers.normalXY.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .deferredGBufferPassOpaque{
                .name{L"GBuffer pass opaque"},
                .type{ePassType::Regular},
                .program{&app.shaders.gBufferPass},
                .blendState{&app.shaders.blendStates.none},
                .samplerStates{app.shaders.samplers.pAnisotropicSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{app.shaders.depthStencilStates.pEqualRead},
                .depthStencilView{app.swapchain.depthStencilView},
                .targets{
                    targets.gbuffers.ambientRG.renderTargetView,
                    targets.gbuffers.diffuseAmbientB.renderTargetView,
                    targets.gbuffers.specularShininess.renderTargetView,
                },
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_RENDER_TARGET},
                .clearValue{DirectX::Colors::White},
            },
            .ssao{
                .name{L"SSAO"},
                .type{ePassType::Compute},
                .program{&app.shaders.ssaoCompute},
                .blendState{&app.shaders.blendStates.none},
                .samplerStates{app.shaders.samplers.pPointSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{
                    app.swapchain.depthStencilShaderResourceView,
                    targets.gbuffers.normalXY.shaderResourceView,
                    targets.noiseTexture.shaderResourceView,
                },
                .depthStencilState{app.shaders.depthStencilStates.pDisable},
                .depthStencilView{nullptr},
                .targets{},
                .targetsCS{targets.ao1.unorderedAccessView},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_FLOAT_UAVS},
                .clearValue{DirectX::Colors::White},
            },
            .ssaoVerticalBlurPass{
                .name{L"SSAO blur vertical"},
                .type{ePassType::Compute},
                .program{&app.shaders.ssaoVerticalBlur},
                .blendState{&app.shaders.blendStates.none},
                .samplerStates{app.shaders.samplers.pBilinearSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{targets.ao1.shaderResourceView},
                .depthStencilState{app.shaders.depthStencilStates.pDisable},
                .depthStencilView{nullptr},
                .targets{},
                .targetsCS{targets.ao2.unorderedAccessView},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .ssaoHorizontalBlurPass{
                .name{L"SSAO blur horizontal"},
                .type{ePassType::Compute},
                .program{&app.shaders.ssaoHorizontalBlur},
                .blendState{&app.shaders.blendStates.none},
                .samplerStates{app.shaders.samplers.pBilinearSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{targets.ao2.shaderResourceView},
                .depthStencilState{app.shaders.depthStencilStates.pDisable},
                .depthStencilView{nullptr},
                .targets{},
                .targetsCS{targets.ao1.unorderedAccessView},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .deferredShadingOpaque{
                .name{L"Deferred shading opaque"},
                .type{ePassType::FullScreen},
                .program{&app.shaders.deferredShading},
                .blendState{&app.shaders.blendStates.none},
                .samplerStates{app.shaders.samplers.pPointSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{
                    app.swapchain.depthStencilShaderResourceView,
                    targets.gbuffers.normalXY.shaderResourceView,
                    targets.gbuffers.ambientRG.shaderResourceView,
                    targets.gbuffers.diffuseAmbientB.shaderResourceView,
                    targets.gbuffers.specularShininess.shaderResourceView,
                    targets.ao1.shaderResourceView,
                },
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{app.shaders.depthStencilStates.pDisable},
                .depthStencilView{nullptr},
                .targets{targets.basePass.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .forwardShadingOpaque{
                .name{L"Forward shading opaque"},
                .type{ePassType::Regular},
                .program{&app.shaders.forwardShading},
                .blendState{&app.shaders.blendStates.none},
                .samplerStates{app.shaders.samplers.pAnisotropicSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{targets.ao1.shaderResourceView},
                .resourceOffsetPS{4},
                .resourcesCS{},
                .depthStencilState{app.shaders.depthStencilStates.pEqualRead},
                .depthStencilView{app.swapchain.depthStencilView},
                .targets{targets.basePass.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_RENDER_TARGET},
                .clearValue{DirectX::Colors::White},
            },
            .forwardShadingTransparent{
                .name{L"Transparent"},
                .type{ePassType::Regular},
                .program{&app.shaders.forwardShading},
                .blendState{&app.shaders.blendStates.none},
                .samplerStates{app.shaders.samplers.pTrilinearSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{targets.ao1.shaderResourceView},
                .resourceOffsetPS{4},
                .resourcesCS{},
                .depthStencilState{app.shaders.depthStencilStates.pEqualRead},
                .depthStencilView{app.swapchain.depthStencilView},
                .targets{targets.basePass.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .forwardShadingTranclucent{
                .name{L"Translucent"},
                .type{ePassType::Regular},
                .program{&app.shaders.translucentPass},
                .blendState{&app.shaders.blendStates.normal},
                .samplerStates{app.shaders.samplers.pTrilinearSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{app.shaders.depthStencilStates.pLessReadWrite},
                .depthStencilView{app.swapchain.depthStencilView},
                .targets{targets.basePass.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .gammaCorrection{
                .name{L"Gamma correction"},
                .type{ePassType::FullScreen},
                .program{&app.shaders.gammaCorrection},
                .blendState{&app.shaders.blendStates.none},
                .samplerStates{app.shaders.samplers.pPointSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{targets.basePass.shaderResourceView},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{app.shaders.depthStencilStates.pDisable},
                .depthStencilView{nullptr},
                .targets{targets.gammaCorrection.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .debug{
                .name{L"Debug"},
                .type{ePassType::FullScreen},
                .program{&app.shaders.debug},
                .blendState{&app.shaders.blendStates.none},
                .samplerStates{app.shaders.samplers.pPointSampler},
                .cbuffers{&app.shaders.cbuffersDevice},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{
                    targets.gammaCorrection.shaderResourceView,
                    app.swapchain.depthStencilShaderResourceView,
                    targets.ao1.shaderResourceView,
                    targets.gbuffers.normalXY.shaderResourceView,
                    targets.gbuffers.ambientRG.shaderResourceView,
                    targets.gbuffers.diffuseAmbientB.shaderResourceView,
                    targets.gbuffers.specularShininess.shaderResourceView,
                },
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{app.shaders.depthStencilStates.pDisable},
                .depthStencilView{nullptr},
                .targets{targets.debug.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .ui{
                .name{L"UI pass"},
                .type{ePassType::None},
                .program{nullptr},
                .blendState{nullptr},
                .samplerStates{},
                .cbuffers{nullptr},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{app.shaders.depthStencilStates.pDisable},
                .depthStencilView{nullptr},
                .targets{targets.debug.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
        };

        return pipeline;
    }

} // namespace h2r
