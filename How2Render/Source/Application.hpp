#pragma once

#include "UserInterface.hpp"
#include "Window.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Shader.hpp"
#include "Wrapper/Swapchain.hpp"

namespace h2r
{

    struct Application
    {
        enum class eShadingType : int32_t
        {
            Forward = 0,
            Deferred = 1,
            Count,
        };

        enum class eFinalOutput : int32_t
        {
            FinalImage = 0,
            Depth,
            Normals,
            AO,
            gBufferAmbient,
            gBufferDiffuse,
            gBufferSpecular,
            gBufferShininess,
            Count,
        };

        struct States
        {
            bool showSideBarWindow = true;
            bool drawOpaque = true;
            bool drawTransparent = true;
            bool drawTranslucent = true;

            bool normalMappingEnabled = true;
            bool ssaoEnabled = true;
            bool ssaoBlurEnabled = true;
            int32_t ssaoKernelSize = 16;
            float ssaoKernelRadius = 1.5f;
            float ssaoBias = 0.0001f;

            eShadingType shadingType = eShadingType::Deferred;
            double shadingGPUTimeMs = 0;

            eFinalOutput finalOutput = eFinalOutput::FinalImage;
        };

        Context context;
        Swapchain swapchain;
        Shaders shaders;
        States states;
    };

    inline Application CreateApplication(Window &window)
    {
        Application app;

        app.context = CreateContext();
        app.swapchain = CreateSwapchain(window, app.context);
        app.shaders = CreateShaders(app.context).value();
        InitUI(window, app.context);

        return app;
    }

    inline void CleanupApplication(Application &application)
    {
        CleanupUI();
        CleanupContext(application.context);
        CleanupSwapchain(application.swapchain);
        CleanupShaders(application.shaders);
    }

} // namespace h2r
