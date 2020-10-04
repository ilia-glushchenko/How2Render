#pragma once

#include "Camera.hpp"
#include "DirectionalLight.hpp"
#include "Helpers/MeshGenerator.hpp"
#include "RenderCommon.hpp"
#include "RenderObject.hpp"
#include "RenderPipeline.hpp"
#include "UserInterface.hpp"
#include "Wrapper/Query.hpp"
#include "Wrapper/Shader.hpp"

namespace h2r
{

    inline RenderObjectStorage LoadRenderObjectStorage(Context const &context, TextureCache &cache)
    {
        HostModel const sponzaHostModel = LoadObjModel("Data\\Models\\sponza\\sponza.obj", cache).value();
        DeviceModel const sponzaDeviceModel = CreateDeviceModel(context, cache, sponzaHostModel);
        RenderObject const sponzaRenderObject = CreateRenderObject(sponzaDeviceModel, {0, 0, 0}, {0, 0, 0}, 0.01f);

        std::vector<RenderObject> opaqueObjects = {sponzaRenderObject};
        std::vector<RenderObject> translucentObjects = GenerateSpheres(context, cache);

        return RenderObjectStorage{opaqueObjects, translucentObjects};
    }

    inline void CleanupRenderObjectStorage(RenderObjectStorage &storage)
    {
        for (auto &object : storage.opaque)
        {
            CleanupRenderObject(object);
        }
        for (auto &object : storage.translucent)
        {
            CleanupRenderObject(object);
        }
    }

    inline void DispatchSSAO(Context const &context, Application::States const &states, Swapchain const &swapchain)
    {
        if (states.ssaoEnabled)
        {
            context.pImmediateContext->Dispatch(swapchain.width / 32 + 1, swapchain.height / 32 + 1, 1);
        }
    }

    inline void DispatchSsaoBlur(Context const &context, Application::States const &states, Swapchain const &swapchain)
    {
        if (states.ssaoBlurEnabled)
        {
            context.pImmediateContext->Dispatch(swapchain.width / 32 + 1, swapchain.height / 32 + 1, 1);
        }
    }

    inline void SortTranslucentRenderObjects(Camera const &camera, std::vector<RenderObject> &translucentObjects)
    {
        std::sort(translucentObjects.begin(), translucentObjects.end(), [&camera](auto const &a, auto const &b) {
            auto const aLengthSq = std::abs(XMVector3LengthSq(a.transform.position - camera.position).m128_f32[0]);
            auto const bLengthSq = std::abs(XMVector3LengthSq(b.transform.position - camera.position).m128_f32[0]);
            return aLengthSq > bLengthSq;
        });
    }

    inline void Present(
        Context const &context,
        Swapchain const &swapchain,
        ID3D11Resource *sourceTexture,
        ID3D11Resource *presentTexture)
    {
        // Blit off screen texture to back buffer
        context.pImmediateContext->CopyResource(presentTexture, sourceTexture);
        swapchain.pSwapChain->Present(0, 0);
    }

    inline void MainLoop()
    {
        Window window = CreateNewWindow(1200, 720);
        Camera camera = CreateDefaultCamera();
        InputEvents inputs = CreateDefaultInputEvents();
        Application app = CreateApplication(window);
        InitUI(window, app.context);

        PerformaceQueries queries = CreatePerformanceQueries(app.context);
        auto cbuffers = CreatePipelineConstBuffers(app.context).value();
        auto states = CreatePipelineStates(app.context).value();
        auto textures = CreatePipelineTextures(app.context, app.swapchain).value();
        auto shaders = CreatePipelineShaders(app.context).value();

        Pipeline pipeline = CreateRenderPipeline(app.swapchain, shaders, cbuffers, states, textures);

        TextureCache textureCache;
        RenderObjectStorage storage = LoadRenderObjectStorage(app.context, textureCache);
        DirectionalLight light = CreateDirectionalLight(app.context);
        UpdateInfrequentConstantBuffer(app.context, app.states, light, cbuffers.device, cbuffers.host.infrequent);

        while (!inputs.quit)
        {
            UpdateInput(inputs);
            if (ReloadePipelineShaders(app.context, inputs, shaders))
            {
                UpdateInfrequentConstantBuffer(app.context, app.states, light, cbuffers.device, cbuffers.host.infrequent);
            }
            UpdateCamera(camera, inputs, window);
            UpdatePerFrameConstantBuffer(app.context, camera, cbuffers.device, cbuffers.host.perFrame);

            BeginQueryGpuTime(app.context, queries);

            // Pre pass
            {
                BindRenderPass(app.context, pipeline.depthPrePassOpaque);
                UpdatePerPassConstantBuffer(app.context, pipeline.depthPrePassOpaque, cbuffers.device, cbuffers.host.perPass);
                DrawOpaqueRenderObjects(app.context, app.states, storage.opaque, cbuffers.device, cbuffers.host);
                UnbindRenderPass(app.context, pipeline.depthPrePassOpaque);

                BindRenderPass(app.context, pipeline.depthPrePassTransparent);
                UpdatePerPassConstantBuffer(app.context, pipeline.depthPrePassTransparent, cbuffers.device, cbuffers.host.perPass);
                DrawTransparentRenderObjects(app.context, app.states, storage.opaque, cbuffers.device, cbuffers.host);
                UnbindRenderPass(app.context, pipeline.depthPrePassTransparent);
            }

            // Shadow depths
            if (app.states.shadowMappingEnabled)
            {
                BindRenderPass(app.context, pipeline.shadowDepthOpaque);
                UpdatePerPassConstantBuffer(app.context, pipeline.shadowDepthOpaque, cbuffers.device, cbuffers.host.perPass);
                DrawOpaqueRenderObjects(app.context, app.states, storage.opaque, cbuffers.device, cbuffers.host);
                UnbindRenderPass(app.context, pipeline.shadowDepthOpaque);

                BindRenderPass(app.context, pipeline.shadowDepthTransparent);
                UpdatePerPassConstantBuffer(app.context, pipeline.shadowDepthTransparent, cbuffers.device, cbuffers.host.perPass);
                DrawTransparentRenderObjects(app.context, app.states, storage.opaque, cbuffers.device, cbuffers.host);
                UnbindRenderPass(app.context, pipeline.shadowDepthTransparent);
            }

            // SSAO
            {
                BindRenderPass(app.context, pipeline.ssao);
                UpdatePerPassConstantBuffer(app.context, pipeline.ssao, cbuffers.device, cbuffers.host.perPass);
                DispatchSSAO(app.context, app.states, app.swapchain);
                UnbindRenderPass(app.context, pipeline.ssao);

                BindRenderPass(app.context, pipeline.ssaoVerticalBlurPass);
                UpdatePerPassConstantBuffer(app.context, pipeline.ssaoVerticalBlurPass, cbuffers.device, cbuffers.host.perPass);
                DispatchSsaoBlur(app.context, app.states, app.swapchain);
                UnbindRenderPass(app.context, pipeline.ssaoVerticalBlurPass);

                BindRenderPass(app.context, pipeline.ssaoHorizontalBlurPass);
                UpdatePerPassConstantBuffer(app.context, pipeline.ssaoHorizontalBlurPass, cbuffers.device, cbuffers.host.perPass);
                DispatchSsaoBlur(app.context, app.states, app.swapchain);
                UnbindRenderPass(app.context, pipeline.ssaoHorizontalBlurPass);
            }

            switch (app.states.shadingType)
            {
            case Application::eShadingType::Forward:
                BindRenderPass(app.context, pipeline.forwardShadingOpaque);
                UpdatePerPassConstantBuffer(app.context, pipeline.forwardShadingOpaque, cbuffers.device, cbuffers.host.perPass);
                DrawOpaqueRenderObjects(app.context, app.states, storage.opaque, cbuffers.device, cbuffers.host);
                UnbindRenderPass(app.context, pipeline.forwardShadingOpaque);
                break;
            case Application::eShadingType::Deferred:
                BindRenderPass(app.context, pipeline.deferredGBufferPassOpaque);
                UpdatePerPassConstantBuffer(app.context, pipeline.deferredGBufferPassOpaque, cbuffers.device, cbuffers.host.perPass);
                DrawOpaqueRenderObjects(app.context, app.states, storage.opaque, cbuffers.device, cbuffers.host);
                UnbindRenderPass(app.context, pipeline.deferredGBufferPassOpaque);

                BindRenderPass(app.context, pipeline.deferredShadingOpaque);
                UpdatePerPassConstantBuffer(app.context, pipeline.deferredShadingOpaque, cbuffers.device, cbuffers.host.perPass);
                DrawFullScreen(app.context);
                UnbindRenderPass(app.context, pipeline.deferredShadingOpaque);
                break;
            default:
                printf("Invalid shading type\n");
                assert(true);
                break;
            }

            BindRenderPass(app.context, pipeline.forwardShadingTransparent);
            UpdatePerPassConstantBuffer(app.context, pipeline.forwardShadingTransparent, cbuffers.device, cbuffers.host.perPass);
            DrawTransparentRenderObjects(app.context, app.states, storage.opaque, cbuffers.device, cbuffers.host);
            UnbindRenderPass(app.context, pipeline.forwardShadingTransparent);

            SortTranslucentRenderObjects(camera, storage.translucent);
            BindRenderPass(app.context, pipeline.forwardShadingTranclucent);
            UpdatePerPassConstantBuffer(app.context, pipeline.forwardShadingTranclucent, cbuffers.device, cbuffers.host.perPass);
            DrawTranslucentRenderObjects(app.context, app.states, storage.translucent, cbuffers.device, cbuffers.host);
            UnbindRenderPass(app.context, pipeline.forwardShadingTranclucent);

            BindRenderPass(app.context, pipeline.gammaCorrection);
            UpdatePerPassConstantBuffer(app.context, pipeline.gammaCorrection, cbuffers.device, cbuffers.host.perPass);
            DrawFullScreen(app.context);
            UnbindRenderPass(app.context, pipeline.gammaCorrection);

            EndQueryGpuTime(app.context, queries);
            app.states.shadingGPUTimeMs = BlockAndGetGpuTimeMs(app.context, queries);

            BindRenderPass(app.context, pipeline.debug);
            UpdatePerPassConstantBuffer(app.context, pipeline.debug, cbuffers.device, cbuffers.host.perPass);
            DrawFullScreen(app.context);
            UnbindRenderPass(app.context, pipeline.debug);

            BindRenderPass(app.context, pipeline.ui);
            UpdatePerPassConstantBuffer(app.context, pipeline.ui, cbuffers.device, cbuffers.host.perPass);
            if (DrawUI(window, app.context, cbuffers.device, cbuffers.host, app.states, camera, light))
            {
                UpdateInfrequentConstantBuffer(app.context, app.states, light, cbuffers.device, cbuffers.host.infrequent);
            }
            UnbindRenderPass(app.context, pipeline.ui);

            Present(app.context, app.swapchain, textures.debug.texture, app.swapchain.renderTargetTexture);
        }

        CleanupUI();
        CleanupPerformanceQueries(queries);
        CleanupPipelineShaders(shaders);
        CleanupPipelineTextures(textures);
        CleanupRenderObjectStorage(storage);
        FlushTextureCache(textureCache);
        CleanupApplication(app);
        CleanupWindow(window);
    }

} // namespace h2r
