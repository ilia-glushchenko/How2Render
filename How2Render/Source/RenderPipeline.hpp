#pragma once

#include "Application.hpp"
#include "RenderPass.hpp"
#include "Wrapper/RasterizerState.hpp"
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

            TextureSamplers samplers;

            DeviceTexture shadowDepth;
            DeviceTexture ao1;
            DeviceTexture ao2;
            GBuffers gbuffers;
            DeviceTexture basePass;
            DeviceTexture gammaCorrection;
            DeviceTexture debug;
            Swapchain swapchain;
            DeviceTexture noiseTexture;
        };

        struct Shaders
        {
            ShaderProgram depthPrePassOpaque;
            ShaderProgram depthPrePassTransparent;

            ShaderProgram shadowDepthOpaque;
            ShaderProgram shadowDepthTransparent;

            ShaderProgram ssaoCompute;
            ShaderProgram ssaoVerticalBlur;
            ShaderProgram ssaoHorizontalBlur;

            ShaderProgram forwardShading;

            ShaderProgram gBufferPass;
            ShaderProgram deferredShading;

            ShaderProgram translucentPass;
            ShaderProgram gammaCorrection;
            ShaderProgram debug;
        };

        struct ConstBuffers
        {
            HostConstBuffers host;
            DeviceConstBuffers device;
        };

        struct States
        {
            BlendStates blend;
            RasterizerStates rasterizer;
            DepthStencilStates depthStencil;
        };

        Pass depthPrePassOpaque;
        Pass depthPrePassTransparent;
        Pass shadowDepthOpaque;
        Pass shadowDepthTransparent;
        Pass ssao;
        Pass ssaoVerticalBlurPass;
        Pass ssaoHorizontalBlurPass;
        Pass deferredGBufferPassOpaque;
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

    inline std::optional<Pipeline::Shaders> CreatePipelineShaders(Context const &context);

    inline void CleanupPipelineShaders(Pipeline::Shaders &shaders);

    inline bool ReloadePipelineShaders(Context const &context, InputEvents const &inputs, Pipeline::Shaders &shaders);

    inline std::optional<Pipeline::ConstBuffers> CreatePipelineConstBuffers(Context const &context);

    inline void CleanupPipelineConstBuffers(Pipeline::ConstBuffers &cbuffers);

    inline std::optional<Pipeline::States> CreatePipelineStates(Context const &context);

    inline void CleanupPipelineStates(Pipeline::States &states);

    inline Pipeline CreateRenderPipeline(
        Swapchain const &swapchain,
        Pipeline::Shaders const &shaders,
        Pipeline::ConstBuffers const &cbuffers,
        Pipeline::States const &states,
        Pipeline::Textures const &textures);

} // namespace h2r

namespace h2r
{

    inline std::optional<Pipeline::Textures::GBuffers> CreateGBuffers(
        Context const &context, uint32_t width, uint32_t height)
    {
        Pipeline::Textures::GBuffers gbuffers;

        {
            auto texture = CreateRenderTargetTexture(context, width, height, DXGI_FORMAT_R8G8_UNORM);
            if (!texture)
            {
                printf("Failed to create Normal GBuffer device texture\n");
                return std::nullopt;
            }
            gbuffers.normalXY = texture.value();
        }
        {
            auto texture = CreateRenderTargetTexture(context, width, height, DXGI_FORMAT_R8G8_UNORM);
            if (!texture)
            {
                printf("Failed to create Ambient GBuffer device texture\n");
                return std::nullopt;
            }
            gbuffers.ambientRG = texture.value();
        }
        {
            auto texture = CreateRenderTargetTexture(context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
            if (!texture)
            {
                printf("Failed to create Diffuse GBuffer device texture\n");
                return std::nullopt;
            }
            gbuffers.diffuseAmbientB = texture.value();
        }
        {
            auto texture = CreateRenderTargetTexture(context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
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
        textures.samplers = CreateTextureSamplers(context);
        textures.swapchain = swapchain;
        textures.gbuffers = CreateGBuffers(context, windowSize.x, windowSize.y).value();

        {
            auto texture = CreateDepthStencilTexture(context, 2048, 2048, eDepthPrecision::Unorm16);
            if (!texture)
            {
                printf("Failed to create shadow depth texture!\n");
                return std::nullopt;
            }
            textures.shadowDepth = texture.value();
        }

        {
            auto texture = CreateComputeTargetTexture(context, windowSize.x, windowSize.y, DXGI_FORMAT_R8_UNORM);
            if (!texture)
            {
                printf("Failed to create ao render target!\n");
                return std::nullopt;
            }
            textures.ao1 = texture.value();
        }

        {
            auto texture = CreateComputeTargetTexture(context, windowSize.x, windowSize.y, DXGI_FORMAT_R8_UNORM);
            if (!texture)
            {
                printf("Failed to create ao render target!\n");
                return std::nullopt;
            }
            textures.ao2 = texture.value();
        }

        {
            auto texture = CreateRenderTargetTexture(context, windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM);
            if (!texture)
            {
                printf("Failed to create shading pass render target!\n");
                return std::nullopt;
            }
            textures.basePass = texture.value();
        }

        {
            auto texture = CreateRenderTargetTexture(context, windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM);
            if (!texture)
            {
                printf("Failed to create gamma correction render target!\n");
                return std::nullopt;
            }
            textures.gammaCorrection = texture.value();
        }

        {
            auto texture = CreateRenderTargetTexture(context, windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM);
            if (!texture)
            {
                printf("Failed to create debug render target!\n");
                return std::nullopt;
            }
            textures.debug = texture.value();
        }

        {
            textures.noiseTexture = GenerateNoiseTexture(context, 16, 16);
        }

        return textures;
    }

    inline void CleanupPipelineTextures(Pipeline::Textures &textures)
    {
        CleanupTextureSamplers(textures.samplers);
        CleanupDeviceTexture(textures.gbuffers.normalXY);
        CleanupDeviceTexture(textures.gbuffers.ambientRG);
        CleanupDeviceTexture(textures.gbuffers.diffuseAmbientB);
        CleanupDeviceTexture(textures.gbuffers.specularShininess);
        CleanupDeviceTexture(textures.shadowDepth);
        CleanupDeviceTexture(textures.ao1);
        CleanupDeviceTexture(textures.ao2);
        CleanupDeviceTexture(textures.basePass);
        CleanupDeviceTexture(textures.gammaCorrection);
        CleanupDeviceTexture(textures.debug);
        CleanupDeviceTexture(textures.noiseTexture);
    }

    inline std::optional<Pipeline::Shaders> CreatePipelineShaders(Context const &context)
    {
        Pipeline::Shaders shaders;

        ShaderProgramDescriptor depthPrePassDescriptor;
        depthPrePassDescriptor.vertexShaderPath = "Shaders/Depth.fx";
        depthPrePassDescriptor.pixelShaderPath = "Shaders/Depth.fx";
        if (auto shader = CreateShaderProgram(context, depthPrePassDescriptor); shader)
        {
            shaders.depthPrePassOpaque = shader.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        depthPrePassDescriptor.definitions = {
            {"ENABLE_TRANSPARENCY", "1"},
            {nullptr, nullptr},
        };
        if (auto shader = CreateShaderProgram(context, depthPrePassDescriptor); shader)
        {
            shaders.depthPrePassTransparent = shader.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        ShaderProgramDescriptor shadowDepthOpaqueDesc;
        shadowDepthOpaqueDesc.vertexShaderPath = "Shaders/ShadowDepth.fx";
        if (auto shader = CreateShaderProgram(context, shadowDepthOpaqueDesc); shader)
        {
            shaders.shadowDepthOpaque = shader.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        ShaderProgramDescriptor shadowDepthTransparentDesc;
        shadowDepthTransparentDesc.vertexShaderPath = "Shaders/ShadowDepth.fx";
        shadowDepthTransparentDesc.pixelShaderPath = "Shaders/ShadowDepth.fx";
        if (auto shader = CreateShaderProgram(context, shadowDepthTransparentDesc); shader)
        {
            shaders.shadowDepthTransparent = shader.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        ShaderProgramDescriptor ssaoComputeDescriptor;
        ssaoComputeDescriptor.computeShaderPath = "Shaders/SSAO.fx";
        ssaoComputeDescriptor.computeShaderEntryPoint = "ComputeSSAO";
        if (auto shader = CreateShaderProgram(context, ssaoComputeDescriptor); shader)
        {
            shaders.ssaoCompute = shader.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        ssaoComputeDescriptor.computeShaderPath = "Shaders/SsaoBlur.fx";
        ssaoComputeDescriptor.computeShaderEntryPoint = "GaussianBlurVertical";
        if (auto shader = CreateShaderProgram(context, ssaoComputeDescriptor); shader)
        {
            shaders.ssaoVerticalBlur = shader.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        ssaoComputeDescriptor.computeShaderEntryPoint = "GaussianBlurHorizontal";
        if (auto shader = CreateShaderProgram(context, ssaoComputeDescriptor); shader)
        {
            shaders.ssaoHorizontalBlur = shader.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        ShaderProgramDescriptor forwardShadingPassDesc;
        forwardShadingPassDesc.vertexShaderPath = "Shaders/ForwardShading.fx";
        forwardShadingPassDesc.pixelShaderPath = "Shaders/ForwardShading.fx";
        if (auto forwardShading = CreateShaderProgram(context, forwardShadingPassDesc); forwardShading)
        {
            shaders.forwardShading = forwardShading.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        ShaderProgramDescriptor gBufferPassDesc;
        gBufferPassDesc.vertexShaderPath = "Shaders/DeferredGBufferPass.fx";
        gBufferPassDesc.pixelShaderPath = "Shaders/DeferredGBufferPass.fx";
        if (auto gBufferPass = CreateShaderProgram(context, gBufferPassDesc); gBufferPass)
        {
            shaders.gBufferPass = gBufferPass.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        ShaderProgramDescriptor deferredShadingPassDesc;
        deferredShadingPassDesc.vertexShaderPath = "Shaders/DeferredShading.fx";
        deferredShadingPassDesc.pixelShaderPath = "Shaders/DeferredShading.fx";
        if (auto deferredShading = CreateShaderProgram(context, deferredShadingPassDesc); deferredShading)
        {
            shaders.deferredShading = deferredShading.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        ShaderProgramDescriptor translucencyPassDesc;
        translucencyPassDesc.vertexShaderPath = "Shaders/Translucent.fx";
        translucencyPassDesc.pixelShaderPath = "Shaders/Translucent.fx";
        if (auto translucentPass = CreateShaderProgram(context, translucencyPassDesc); translucentPass)
        {
            shaders.translucentPass = translucentPass.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        ShaderProgramDescriptor gammaCorrectionDesc;
        gammaCorrectionDesc.vertexShaderPath = "Shaders/GammaCorrection.fx";
        gammaCorrectionDesc.pixelShaderPath = "Shaders/GammaCorrection.fx";
        if (auto gammaCorrection = CreateShaderProgram(context, gammaCorrectionDesc); gammaCorrection)
        {
            shaders.gammaCorrection = gammaCorrection.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        ShaderProgramDescriptor debugDesc;
        debugDesc.vertexShaderPath = "Shaders/Debug.fx";
        debugDesc.pixelShaderPath = "Shaders/Debug.fx";
        if (auto shader = CreateShaderProgram(context, debugDesc); shader)
        {
            shaders.debug = shader.value();
        }
        else
        {
            CleanupPipelineShaders(shaders);
            return std::nullopt;
        }

        return shaders;
    }

    inline void CleanupPipelineShaders(Pipeline::Shaders &shaders)
    {
        CleanupShaderProgram(shaders.depthPrePassOpaque);
        CleanupShaderProgram(shaders.depthPrePassTransparent);
        CleanupShaderProgram(shaders.shadowDepthOpaque);
        CleanupShaderProgram(shaders.shadowDepthTransparent);
        CleanupShaderProgram(shaders.ssaoCompute);
        CleanupShaderProgram(shaders.ssaoVerticalBlur);
        CleanupShaderProgram(shaders.ssaoHorizontalBlur);
        CleanupShaderProgram(shaders.forwardShading);
        CleanupShaderProgram(shaders.gBufferPass);
        CleanupShaderProgram(shaders.deferredShading);
        CleanupShaderProgram(shaders.translucentPass);
        CleanupShaderProgram(shaders.gammaCorrection);
        CleanupShaderProgram(shaders.debug);
    }

    inline bool ReloadePipelineShaders(Context const &context, InputEvents const &inputs, Pipeline::Shaders &shaders)
    {
        if (IsKeyPressed(inputs, SDL_SCANCODE_F5))
        {
            if (auto newShaders = CreatePipelineShaders(context); newShaders)
            {
                CleanupPipelineShaders(shaders);
                shaders = newShaders.value();
                printf("Shader hot reload succeeded\n");
                return true;
            }

            printf("Shader hot reload failed\n");
        }

        return false;
    }

    inline std::optional<Pipeline::ConstBuffers> CreatePipelineConstBuffers(Context const &context)
    {
        Pipeline::ConstBuffers cbuffers;

        auto cbuffersDevice = CreateDeviceConstantBuffers(context);
        if (!cbuffersDevice)
        {
            printf("Failed to create device const buffers.\n");
            return std::nullopt;
        }
        cbuffers.device = cbuffersDevice.value();
        cbuffers.host = CreateHostConstBuffers();

        return cbuffers;
    }

    inline void CleanupPipelineConstBuffers(Pipeline::ConstBuffers &cbuffers)
    {
        CleanupDeviceConstantBuffers(cbuffers.device);
    }

    inline std::optional<Pipeline::States> CreatePipelineStates(Context const &context)
    {
        Pipeline::States states;

        {
            auto state = CreateBlendStates(context);
            if (!state)
            {
                printf("Failed to create blend states.\n");
                return std::nullopt;
            }
            states.blend = state.value();
        }

        {
            auto state = CreateRasterizerStates(context);
            if (!state)
            {
                printf("Failed to create rasterizer states.\n");
                CleanupPipelineStates(states);
                return std::nullopt;
            }
            states.rasterizer = state.value();
        }

        {
            auto state = CreateDepthStencilStates(context);
            if (!state)
            {
                printf("Failed to create depth stencil states.\n");
                CleanupPipelineStates(states);
                return std::nullopt;
            }
            states.depthStencil = state.value();
        }

        return states;
    }

    inline void CleanupPipelineStates(Pipeline::States &states)
    {
        CleanupBlendStates(states.blend);
        CleanupRasterizerStates(states.rasterizer);
        CleanupDepthStencilStates(states.depthStencil);
    }

    inline Pipeline CreateRenderPipeline(
        Swapchain const &swapchain,
        Pipeline::Shaders const &shaders,
        Pipeline::ConstBuffers const &cbuffers,
        Pipeline::States const &states,
        Pipeline::Textures const &textures)
    {
        return Pipeline{
            .depthPrePassOpaque{
                .name{L"Depth pre-pass opaque"},
                .type{ePassType::Regular},
                .viewportSize{textures.gbuffers.normalXY.width, textures.gbuffers.normalXY.height},
                .program{&shaders.depthPrePassOpaque},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{textures.samplers.pTrilinearSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pLessReadWrite},
                .depthStencilView{swapchain.depthStencilView},
                .targets{textures.gbuffers.normalXY.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_DEPTH_STENCIL | RENDER_PASS_CLEAR_FLAG_RENDER_TARGET},
                .clearValue{DirectX::Colors::Black},
            },
            .depthPrePassTransparent{
                .name{L"Depth pre-pass transparent"},
                .type{ePassType::Regular},
                .viewportSize{textures.gbuffers.normalXY.width, textures.gbuffers.normalXY.height},
                .program{&shaders.depthPrePassTransparent},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{textures.samplers.pTrilinearSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pLessReadWrite},
                .depthStencilView{swapchain.depthStencilView},
                .targets{textures.gbuffers.normalXY.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .shadowDepthOpaque{
                .name{L"Shadow depth opaque"},
                .type{ePassType::Regular},
                .viewportSize{textures.shadowDepth.width, textures.shadowDepth.height},
                .program{&shaders.shadowDepthOpaque},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.shadowDepth},
                .samplerStates{textures.samplers.pPointSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pLessReadWrite},
                .depthStencilView{textures.shadowDepth.depthStencilView},
                .targets{},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_DEPTH_STENCIL},
                .clearValue{DirectX::Colors::Black},
            },
            .shadowDepthTransparent{
                .name{L"Shadow depth transparent"},
                .type{ePassType::Regular},
                .viewportSize{textures.shadowDepth.width, textures.shadowDepth.height},
                .program{&shaders.shadowDepthTransparent},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.shadowDepth},
                .samplerStates{textures.samplers.pPointSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pLessReadWrite},
                .depthStencilView{textures.shadowDepth.depthStencilView},
                .targets{},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .ssao{
                .name{L"SSAO"},
                .type{ePassType::Compute},
                .viewportSize{textures.ao1.width, textures.ao1.height},
                .program{&shaders.ssaoCompute},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{textures.samplers.pPointSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{
                    swapchain.depthStencilShaderResourceView,
                    textures.gbuffers.normalXY.shaderResourceView,
                    textures.noiseTexture.shaderResourceView,
                },
                .depthStencilState{states.depthStencil.pDisable},
                .depthStencilView{nullptr},
                .targets{},
                .targetsCS{textures.ao1.unorderedAccessView},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_FLOAT_UAVS},
                .clearValue{DirectX::Colors::White},
            },
            .ssaoVerticalBlurPass{
                .name{L"SSAO blur vertical"},
                .type{ePassType::Compute},
                .viewportSize{textures.ao2.width, textures.ao2.height},
                .program{&shaders.ssaoVerticalBlur},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{textures.samplers.pBilinearSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{textures.ao1.shaderResourceView},
                .depthStencilState{states.depthStencil.pDisable},
                .depthStencilView{nullptr},
                .targets{},
                .targetsCS{textures.ao2.unorderedAccessView},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .ssaoHorizontalBlurPass{
                .name{L"SSAO blur horizontal"},
                .type{ePassType::Compute},
                .viewportSize{textures.ao1.width, textures.ao1.height},
                .program{&shaders.ssaoHorizontalBlur},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{textures.samplers.pBilinearSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{textures.ao2.shaderResourceView},
                .depthStencilState{states.depthStencil.pDisable},
                .depthStencilView{nullptr},
                .targets{},
                .targetsCS{textures.ao1.unorderedAccessView},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .deferredGBufferPassOpaque{
                .name{L"GBuffer pass opaque"},
                .type{ePassType::Regular},
                .viewportSize{textures.basePass.width, textures.basePass.height},
                .program{&shaders.gBufferPass},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{textures.samplers.pAnisotropicSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pEqualRead},
                .depthStencilView{swapchain.depthStencilView},
                .targets{
                    textures.gbuffers.ambientRG.renderTargetView,
                    textures.gbuffers.diffuseAmbientB.renderTargetView,
                    textures.gbuffers.specularShininess.renderTargetView,
                },
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_RENDER_TARGET},
                .clearValue{DirectX::Colors::White},
            },
            .deferredShadingOpaque{
                .name{L"Deferred shading opaque"},
                .type{ePassType::FullScreen},
                .viewportSize{textures.basePass.width, textures.basePass.height},
                .program{&shaders.deferredShading},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{textures.samplers.pPointSampler, textures.samplers.pDepthComparationSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{
                    swapchain.depthStencilShaderResourceView,
                    textures.gbuffers.normalXY.shaderResourceView,
                    textures.gbuffers.ambientRG.shaderResourceView,
                    textures.gbuffers.diffuseAmbientB.shaderResourceView,
                    textures.gbuffers.specularShininess.shaderResourceView,
                    textures.ao1.shaderResourceView,
                    textures.shadowDepth.shaderResourceView,
                    textures.noiseTexture.shaderResourceView,
                },
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pDisable},
                .depthStencilView{nullptr},
                .targets{textures.basePass.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .forwardShadingOpaque{
                .name{L"Forward shading opaque"},
                .type{ePassType::Regular},
                .viewportSize{textures.basePass.width, textures.basePass.height},
                .program{&shaders.forwardShading},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{
                    textures.samplers.pPointSampler,
                    textures.samplers.pAnisotropicSampler,
                    textures.samplers.pDepthComparationSampler,
                },
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{
                    textures.ao1.shaderResourceView,
                    textures.shadowDepth.shaderResourceView,
                    textures.noiseTexture.shaderResourceView},
                .resourceOffsetPS{MaterialTextureCount},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pEqualRead},
                .depthStencilView{swapchain.depthStencilView},
                .targets{textures.basePass.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_RENDER_TARGET},
                .clearValue{DirectX::Colors::White},
            },
            .forwardShadingTransparent{
                .name{L"Transparent"},
                .type{ePassType::Regular},
                .viewportSize{textures.basePass.width, textures.basePass.height},
                .program{&shaders.forwardShading},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{
                    textures.samplers.pPointSampler,
                    textures.samplers.pAnisotropicSampler,
                    textures.samplers.pDepthComparationSampler,
                },
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{
                    textures.ao1.shaderResourceView,
                    textures.shadowDepth.shaderResourceView,
                    textures.noiseTexture.shaderResourceView,
                },
                .resourceOffsetPS{MaterialTextureCount},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pEqualRead},
                .depthStencilView{swapchain.depthStencilView},
                .targets{textures.basePass.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .forwardShadingTranclucent{
                .name{L"Translucent"},
                .type{ePassType::Regular},
                .viewportSize{textures.basePass.width, textures.basePass.height},
                .program{&shaders.translucentPass},
                .blendState{&states.blend.normal},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{textures.samplers.pTrilinearSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pLessReadWrite},
                .depthStencilView{swapchain.depthStencilView},
                .targets{textures.basePass.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .gammaCorrection{
                .name{L"Gamma correction"},
                .type{ePassType::FullScreen},
                .viewportSize{textures.basePass.width, textures.basePass.height},
                .program{&shaders.gammaCorrection},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{textures.samplers.pPointSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{textures.basePass.shaderResourceView},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pDisable},
                .depthStencilView{nullptr},
                .targets{textures.gammaCorrection.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .debug{
                .name{L"Debug"},
                .type{ePassType::FullScreen},
                .viewportSize{textures.basePass.width, textures.basePass.height},
                .program{&shaders.debug},
                .blendState{&states.blend.none},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{textures.samplers.pPointSampler},
                .cbuffers{&cbuffers.device},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{
                    textures.gammaCorrection.shaderResourceView,
                    swapchain.depthStencilShaderResourceView,
                    textures.ao1.shaderResourceView,
                    textures.shadowDepth.shaderResourceView,
                    textures.gbuffers.normalXY.shaderResourceView,
                    textures.gbuffers.ambientRG.shaderResourceView,
                    textures.gbuffers.diffuseAmbientB.shaderResourceView,
                    textures.gbuffers.specularShininess.shaderResourceView,
                },
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pDisable},
                .depthStencilView{nullptr},
                .targets{textures.debug.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
            .ui{
                .name{L"UI pass"},
                .type{ePassType::None},
                .viewportSize{textures.basePass.width, textures.basePass.height},
                .program{nullptr},
                .blendState{nullptr},
                .rasterizerState{states.rasterizer.defaultRS},
                .samplerStates{},
                .cbuffers{nullptr},
                .resourcesVS{},
                .resourceOffsetVS{0},
                .resourcesPS{},
                .resourceOffsetPS{0},
                .resourcesCS{},
                .depthStencilState{states.depthStencil.pDisable},
                .depthStencilView{nullptr},
                .targets{textures.debug.renderTargetView},
                .targetsCS{},
                .clearFlags{RENDER_PASS_CLEAR_FLAG_NONE},
                .clearValue{DirectX::Colors::Black},
            },
        };
    }

} // namespace h2r
