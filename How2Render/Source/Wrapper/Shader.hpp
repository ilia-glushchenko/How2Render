#pragma once

#include "Helpers/ShaderLoader.hpp"
#include "Input.hpp"
#include "Wrapper/BlendState.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include "Wrapper/DepthStencilState.hpp"
#include "Wrapper/Sampler.hpp"
#include <d3d11.h>
#include <filesystem>

namespace h2r
{

    char const *const g_computeShaderEntryPoint = "CS";
    char const *const g_computeShaderModel = "cs_5_0";
    char const *const g_vertexShaderEntryPoint = "VS";
    char const *const g_vertexShaderModel = "vs_5_0";
    char const *const g_pixelShaderEntryPoint = "PS";
    char const *const g_pixelShaderModel = "ps_5_0";

    struct ShaderProgramDescriptor
    {
        char const *computeShaderEntryPoint = g_computeShaderEntryPoint;
        std::filesystem::path computeShaderPath;
        char const *vertexShaderEntryPoint = g_vertexShaderEntryPoint;
        std::filesystem::path vertexShaderPath;
        char const *pixelShaderEntryPoint = g_pixelShaderEntryPoint;
        std::filesystem::path pixelShaderPath;
        std::vector<D3D_SHADER_MACRO> definitions;
    };

    struct ShaderProgram
    {
        ID3D11ComputeShader *pComputeShader = nullptr;
        ID3D11VertexShader *pVertexShader = nullptr;
        ID3D11PixelShader *pPixelShader = nullptr;
        ID3D11InputLayout *pVertexLayout = nullptr;
    };

    struct ComputeShaderDescriptor
    {
        std::filesystem::path computeShaderPath;
    };

    inline std::optional<ShaderProgram> CreateShaderProgram(Context const &context, ShaderProgramDescriptor const &desc);

    inline void CleanupShaderProgram(ShaderProgram &shaders);

    inline void BindShaders(Context const &context, ShaderProgram const &shaders);

 } // namespace h2r

namespace h2r
{

    inline std::optional<ShaderProgram> CreateShaderProgram(Context const &context, ShaderProgramDescriptor const &desc)
    {
        ShaderProgram shaders = {};
        D3D_SHADER_MACRO const *definitions = desc.definitions.empty() ? nullptr : desc.definitions.data();

        // Create optional compute shader
        if (desc.computeShaderPath.has_filename())
        {
            ID3DBlob *pCSBlob = nullptr;
            auto hr = CompileShaderFromFile(
                desc.computeShaderPath.c_str(), definitions, desc.computeShaderEntryPoint, g_computeShaderModel, &pCSBlob);
            if (FAILED(hr))
            {
                wprintf(L"Failed to compile compute shader from file: '%s'\n", desc.computeShaderPath.c_str());
                return std::nullopt;
            }

            hr = context.pd3dDevice->CreateComputeShader(
                pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), nullptr, &shaders.pComputeShader);
            pCSBlob->Release();
            if (FAILED(hr))
            {
                printf("Failed to create compute shader\n");
                return std::nullopt;
            }
        }
        else
        {
            // Create vertex shader and set up vertex layout
            {
                ID3DBlob *pVSBlob = nullptr;
                auto hr = CompileShaderFromFile(
                    desc.vertexShaderPath.c_str(), definitions, desc.vertexShaderEntryPoint, g_vertexShaderModel, &pVSBlob);
                if (FAILED(hr))
                {
                    printf("Failed to compile vertex shader from file\n");
                    return std::nullopt;
                }

                hr = context.pd3dDevice->CreateVertexShader(
                    pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &shaders.pVertexShader);
                if (FAILED(hr))
                {
                    printf("Failed to create vertex shader\n");
                    pVSBlob->Release();
                    return std::nullopt;
                }

                D3D11_INPUT_ELEMENT_DESC const layout[] =
                    {
                        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
                    };
                hr = context.pd3dDevice->CreateInputLayout(
                    layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &shaders.pVertexLayout);
                pVSBlob->Release();

                if (FAILED(hr))
                {
                    printf("Failed to create input layout\n");
                    CleanupShaderProgram(shaders);
                    return std::nullopt;
                }
                context.pImmediateContext->IASetInputLayout(shaders.pVertexLayout);
            }

            // Create pixel shader
            if (desc.pixelShaderPath.has_filename())
            {
                ID3DBlob *pPSBlob = nullptr;
                auto hr = CompileShaderFromFile(
                    desc.pixelShaderPath.c_str(), definitions, desc.pixelShaderEntryPoint, g_pixelShaderModel, &pPSBlob);
                if (FAILED(hr))
                {
                    wprintf(L"Failed to compile pixel shader from file: '%s'\n", desc.pixelShaderPath.c_str());
                    pPSBlob->Release();
                    CleanupShaderProgram(shaders);
                    return std::nullopt;
                }

                hr = context.pd3dDevice->CreatePixelShader(
                    pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &shaders.pPixelShader);
                pPSBlob->Release();

                if (FAILED(hr))
                {
                    printf("Failed to create pixel shader\n");
                    CleanupShaderProgram(shaders);
                    return std::nullopt;
                }
            }
        }

        return shaders;
    }

    inline void CleanupShaderProgram(ShaderProgram &shaders)
    {
        if (shaders.pComputeShader != nullptr)
        {
            assert(!shaders.pComputeShader->Release());
            shaders.pComputeShader = nullptr;
        }
        if (shaders.pVertexShader != nullptr)
        {
            assert(!shaders.pVertexShader->Release());
            shaders.pVertexShader = nullptr;
        }
        if (shaders.pPixelShader != nullptr)
        {
            assert(!shaders.pPixelShader->Release());
            shaders.pPixelShader = nullptr;
        }
        if (shaders.pVertexLayout != nullptr)
        {
            assert(!shaders.pVertexLayout->Release());
            shaders.pVertexLayout = nullptr;
        }
    };

    inline void BindShaders(Context const &context, ShaderProgram const &shaders)
    {
        context.pImmediateContext->VSSetShader(shaders.pVertexShader, nullptr, 0);
        context.pImmediateContext->PSSetShader(shaders.pPixelShader, nullptr, 0);
        context.pImmediateContext->CSSetShader(shaders.pComputeShader, nullptr, 0);
    }

    inline void UnbindShaders(Context const &context)
    {
        context.pImmediateContext->VSSetShader(nullptr, nullptr, 0);
        context.pImmediateContext->PSSetShader(nullptr, nullptr, 0);
        context.pImmediateContext->CSSetShader(nullptr, nullptr, 0);
    }

} // namespace h2r
