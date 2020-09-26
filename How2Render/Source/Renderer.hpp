#pragma once

#include "Camera.hpp"
#include "Helpers/MeshGenerator.hpp"
#include "RenderCommon.hpp"
#include "RenderObject.hpp"
#include "RenderPipeline.hpp"
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
            auto const aLength = std::abs(XMVector3Length(a.transform.position - camera.position).m128_f32[0]);
            auto const bLength = std::abs(XMVector3Length(b.transform.position - camera.position).m128_f32[0]);
            return aLength > bLength;
        });
    }

    inline void DrawUI(
        Window const &window,
        Context const &context,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers &cbuffersHost,
        Application::States &states,
        Camera &camera)
    {
        static char const *s_shadingTypeNames[static_cast<uint32_t>(Application::eShadingType::Count)] = {
            "Forward Shading",
            "Deferred Shading",
        };

        static char const *s_outputTypeNames[static_cast<uint32_t>(Application::eFinalOutput::Count)] = {
            "Final image",
            "Depth",
            "Normals",
            "AO",
            "GBuffer Ambient",
            "GBuffer Diffuse",
            "GBuffer Specular",
            "GBuffer Shininess"};

        BeginDrawUI(window);

        bool isInputChanged = false;
        if (states.showSideBarWindow)
        {
            ImGui::Begin("Settings", &states.showSideBarWindow);
            ImGui::Text("Camera position");
            isInputChanged &= ImGui::InputFloat3("Position", camera.position.m128_f32, 2);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            isInputChanged |= ImGui::Checkbox("Draw opaque", &states.drawOpaque);
            isInputChanged |= ImGui::Checkbox("Draw transparent", &states.drawTransparent);
            isInputChanged |= ImGui::Checkbox("Draw translucent", &states.drawTranslucent);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            isInputChanged |= ImGui::Checkbox("SSAO", &states.ssaoEnabled);
            isInputChanged |= ImGui::Checkbox("SSAO blur", &states.ssaoBlurEnabled);
            isInputChanged |= ImGui::SliderInt("SSAO kernel", &states.ssaoKernelSize, 1, g_ssaoKernelSize);
            isInputChanged |= ImGui::SliderFloat("SSAO radius", &states.ssaoKernelRadius, 0.1f, 3.f, "%.2f", 1);
            isInputChanged |= ImGui::InputFloat("SSAO bias", &states.ssaoBias, 1e-5f, 1e-2f, 5);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            isInputChanged |= ImGui::Combo(
                "Shading Type",
                reinterpret_cast<int *>(&states.shadingType),
                s_shadingTypeNames,
                static_cast<uint32_t>(Application::eShadingType::Count));
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            isInputChanged |= ImGui::Combo(
                "Output",
                reinterpret_cast<int *>(&states.finalOutput),
                s_outputTypeNames,
                static_cast<uint32_t>(
                    states.shadingType == Application::eShadingType::Deferred 
                    ? Application::eFinalOutput::Count
                    : Application::eFinalOutput::gBufferAmbient
                )
            );
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Shading GPU time: %0.2f ms", states.shadingGPUTimeMs);
            ImGui::End();
        }

        if (isInputChanged)
        {
            UpdateInfrequentConstantBuffers(context, states, cbuffersDevice, cbuffersHost);
        }

        EndDrawUI();
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
        PerformaceQueries queries = CreatePerformanceQueries(app.context);

        Pipeline::Textures pipelineTextures = CreatePipelineTextures(app.context, app.swapchain).value();
        Pipeline pipeline = CreatePipeline(app, pipelineTextures);

        TextureCache textureCache;
        RenderObjectStorage storage = LoadRenderObjectStorage(app.context, textureCache);
        UpdateInfrequentConstantBuffers(app.context, app.states, app.shaders.cbuffersDevice, app.shaders.cbuffersHost);

        while (!inputs.quit)
        {
            UpdateInput(inputs);
            if (HotReloadeShaders(app.context, inputs, app.shaders))
            {
                UpdateInfrequentConstantBuffers(app.context, app.states, app.shaders.cbuffersDevice, app.shaders.cbuffersHost);
            }
            UpdateCamera(camera, inputs, window);
            UpdatePerFrameConstantBuffers(app.context, camera, app.shaders.cbuffersDevice, app.shaders.cbuffersHost);

            BeginQueryGpuTime(app.context, queries);

            // Pre pass
            {
                BindRenderPass(app.context, pipeline.depthPrePassOpaque);
                DrawOpaqueRenderObjects(app.context, app.states, storage.opaque, app.shaders.cbuffersDevice, app.shaders.cbuffersHost);
                UnbindRenderPass(app.context, pipeline.depthPrePassOpaque);

                BindRenderPass(app.context, pipeline.depthPrePassTransparent);
                DrawTransparentRenderObjects(app.context, app.states, storage.opaque, app.shaders.cbuffersDevice, app.shaders.cbuffersHost);
                UnbindRenderPass(app.context, pipeline.depthPrePassTransparent);
            }

            // SSAO
            {
                BindRenderPass(app.context, pipeline.ssao);
                DispatchSSAO(app.context, app.states, app.swapchain);
                UnbindRenderPass(app.context, pipeline.ssao);

                BindRenderPass(app.context, pipeline.ssaoVerticalBlurPass);
                DispatchSsaoBlur(app.context, app.states, app.swapchain);
                UnbindRenderPass(app.context, pipeline.ssaoVerticalBlurPass);

                BindRenderPass(app.context, pipeline.ssaoHorizontalBlurPass);
                DispatchSsaoBlur(app.context, app.states, app.swapchain);
                UnbindRenderPass(app.context, pipeline.ssaoHorizontalBlurPass);
            }

            switch (app.states.shadingType)
            {
            case Application::eShadingType::Forward:
                BindRenderPass(app.context, pipeline.forwardShadingOpaque);
                DrawOpaqueRenderObjects(app.context, app.states, storage.opaque, app.shaders.cbuffersDevice, app.shaders.cbuffersHost);
                UnbindRenderPass(app.context, pipeline.forwardShadingOpaque);
                break;
            case Application::eShadingType::Deferred:
                BindRenderPass(app.context, pipeline.deferredGBufferPassOpaque);
                DrawOpaqueRenderObjects(app.context, app.states, storage.opaque, app.shaders.cbuffersDevice, app.shaders.cbuffersHost);
                UnbindRenderPass(app.context, pipeline.deferredGBufferPassOpaque);

                BindRenderPass(app.context, pipeline.deferredShadingOpaque);
                DrawFullScreen(app.context);
                UnbindRenderPass(app.context, pipeline.deferredShadingOpaque);
                break;
            default:
                printf("Invalid shading type\n");
                assert(true);
                break;
            }

            BindRenderPass(app.context, pipeline.forwardShadingTransparent);
            DrawTransparentRenderObjects(app.context, app.states, storage.opaque, app.shaders.cbuffersDevice, app.shaders.cbuffersHost);
            UnbindRenderPass(app.context, pipeline.forwardShadingTransparent);

            SortTranslucentRenderObjects(camera, storage.translucent);
            BindRenderPass(app.context, pipeline.forwardShadingTranclucent);
            DrawTranslucentRenderObjects(app.context, app.states, storage.translucent, app.shaders.cbuffersDevice, app.shaders.cbuffersHost);
            UnbindRenderPass(app.context, pipeline.forwardShadingTranclucent);

            BindRenderPass(app.context, pipeline.gammaCorrection);
            DrawFullScreen(app.context);
            UnbindRenderPass(app.context, pipeline.gammaCorrection);

            EndQueryGpuTime(app.context, queries);
            app.states.shadingGPUTimeMs = BlockAndGetGpuTimeMs(app.context, queries);

            BindRenderPass(app.context, pipeline.debug);
            DrawFullScreen(app.context);
            UnbindRenderPass(app.context, pipeline.debug);

            BindRenderPass(app.context, pipeline.ui);
            DrawUI(window, app.context, app.shaders.cbuffersDevice, app.shaders.cbuffersHost, app.states, camera);
            UnbindRenderPass(app.context, pipeline.ui);

            Present(app.context, app.swapchain, pipelineTextures.debug.texture, app.swapchain.renderTargetTexture);
        }

        CleanupPerformanceQueries(queries);
        CleanupPipelineTextures(pipelineTextures);
        CleanupRenderObjectStorage(storage);
        FlushTextureCache(textureCache);
        CleanupApplication(app);
        CleanupWindow(window);
    }

} // namespace h2r
