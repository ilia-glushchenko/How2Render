#pragma once

#include "Application.hpp"
#include "Camera.hpp"
#include "DirectionalLight.hpp"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_impl_dx11.h"
#include "ThirdParty/imgui/imgui_impl_sdl.h"
#include "Window.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include "Wrapper/Context.hpp"

namespace h2r
{

    inline void InitUI(Window const &window, Context const &context);

    inline bool DrawUI(
        Window const &window,
        Context const &context,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers &cbuffersHost,
        Application::States &states,
        Camera &camera,
        DirectionalLight &light);

    inline void CleanupUI();

} // namespace h2r

namespace h2r
{

    inline void InitUI(Window const &window, Context const &context)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_InitForD3D(window.pWindow);
        ImGui_ImplDX11_Init(context.pd3dDevice, context.pImmediateContext);

        // ImGui can draw cursors for us on platforms that don't have those.
        // We don't want that here, since Windows has it's own native cursor.
        auto &io = ImGui::GetIO();
        io.MouseDrawCursor = false;
    }

    inline void BeginDrawUI(Window const &window)
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplSDL2_NewFrame(window.pWindow);
        ImGui::NewFrame();
    }

    inline void EndDrawUI()
    {
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    inline void CleanupUI()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    inline bool DrawUI(
        Window const &window,
        Context const &context,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers &cbuffersHost,
        Application::States &states,
        Camera &camera,
        DirectionalLight &light)
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
            "Shadow map",
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
            isInputChanged |= ImGui::InputFloat3("Position", camera.position.m128_f32, 2);
            ImGui::Text("Direct light");
            isInputChanged |= ImGui::InputFloat3("Direction", light.direction.m128_f32, 2);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            isInputChanged |= ImGui::Checkbox("Draw opaque", &states.drawOpaque);
            isInputChanged |= ImGui::Checkbox("Draw transparent", &states.drawTransparent);
            isInputChanged |= ImGui::Checkbox("Draw translucent", &states.drawTranslucent);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            isInputChanged |= ImGui::Checkbox("Normal Mapping", &states.normalMappingEnabled);

            isInputChanged |= ImGui::Checkbox("SSAO", &states.ssaoEnabled);
            isInputChanged |= ImGui::Checkbox("SSAO blur", &states.ssaoBlurEnabled);
            isInputChanged |= ImGui::SliderInt("SSAO kernel", &states.ssaoKernelSize, 1, g_ssaoKernelSize);
            isInputChanged |= ImGui::SliderFloat("SSAO radius", &states.ssaoKernelRadius, 0.1f, 3.f, "%.2f", 1);
            isInputChanged |= ImGui::InputFloat("SSAO bias", &states.ssaoBias, 1e-5f, 1e-2f, 5);

            isInputChanged |= ImGui::Checkbox("Shadow Mapping", &states.shadowMappingEnabled);
            isInputChanged |= ImGui::InputFloat("Shadow bias", &states.shadowMappingBias, 1e-2f, 1e-1f, 3);
            isInputChanged |= ImGui::Checkbox("PCF", &states.pcfEnabled);
            isInputChanged |= ImGui::SliderInt("PCF kernel", &states.pcfKernelSize, 1, g_pcfKernelSize);
            isInputChanged |= ImGui::SliderFloat("PCF radius", &states.pcfRadius, 0.1f, 10.f, "%.2f", 1);
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
                        : Application::eFinalOutput::gBufferAmbient));
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Shading GPU time: %0.2f ms", states.shadingGPUTimeMs);
            ImGui::End();
        }

        EndDrawUI();

        return isInputChanged;
    }

} // namespace h2r