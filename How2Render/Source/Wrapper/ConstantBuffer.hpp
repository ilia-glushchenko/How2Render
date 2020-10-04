#pragma once

#include "Math.hpp"
#include "Random.hpp"
#include "Wrapper/Context.hpp"
#include <cassert>

namespace h2r
{

    constexpr uint32_t g_ssaoKernelSize = 64;
    constexpr uint32_t g_pcfKernelSize = 16;

    struct HostConstBuffers
    {
        struct Transform
        {
            XMMATRIX worldMatrix = {};
        };

        struct Material
        {
            XMFLOAT3 ambient = {};
            float padd0 = 0;
            XMFLOAT3 diffuse = {};
            float padd1 = 0;
            XMFLOAT3 specular = {};
            float shininess = 0;

            float alpha = 0;
            uint32_t normalMapAvailabled = 0;
            XMFLOAT2 padd2 = {};
        };

        struct RenderTarget
        {
            uint32_t width = 0;
            uint32_t height = 0;
            XMFLOAT2 padd = {};
        };

        struct Camera
        {
            XMMATRIX viewMatrix = {};
            XMMATRIX projMatrix = {};
            XMMATRIX invViewMatrix = {};
            XMMATRIX invProjMatrix = {};
            XMVECTOR positionVector = {};
        };

        struct Lights
        {
            XMMATRIX viewProj = {};
            XMVECTOR viewDir = {};
        };

        struct SSAO
        {
            uint32_t kernelSize = 0;
            float kernelRadius = 0;
            float bias = 0;
            XMVECTOR kernel[g_ssaoKernelSize] = {};
        };

        struct Shadows
        {
            uint32_t pcfEnabled = 0;
            uint32_t pcfKernelSize = 0;
            float pcfRadius = 0;
            float bias = 0;
        };

        struct Debug
        {
            uint32_t normalMappingEnabled = 0;
            uint32_t shadowMappingEnabled = 0;
            uint32_t finalOutputIndex = 0;
            float padd0 = 0;
        };

        struct PerInstance
        {
            HostConstBuffers::Transform transform;
        };

        struct PerMaterial
        {
            HostConstBuffers::Material material;
        };

        struct PerPass
        {
            HostConstBuffers::RenderTarget renderTarget;
        };

        struct PerFrame
        {
            HostConstBuffers::Camera camera;
        };

        struct Infrequent
        {
            HostConstBuffers::Lights lights;
            HostConstBuffers::SSAO ssao;
            HostConstBuffers::Shadows shadows;
            HostConstBuffers::Debug debug;
        };

        PerInstance perInstance;
        PerMaterial perMaterial;
        PerPass perPass;
        PerFrame perFrame;
        Infrequent infrequent;
    };

    struct DeviceConstBuffers
    {
        ID3D11Buffer *pPerInstance = nullptr;
        ID3D11Buffer *pPerMaterial = nullptr;
        ID3D11Buffer *pPerPass = nullptr;
        ID3D11Buffer *pPerFrame = nullptr;
        ID3D11Buffer *pInfrequent = nullptr;
    };

    inline HostConstBuffers CreateHostConstBuffers();


    inline std::optional<DeviceConstBuffers> CreateDeviceConstantBuffers(Context const &context);

    inline void CleanupDeviceConstantBuffers(DeviceConstBuffers &cbuffers);


    inline void BindConstantBuffers(Context const &context, DeviceConstBuffers const &cbuffers);

    inline void UnbindConstantBuffer(Context const &context);

} // namespace h2r

namespace h2r
{

    inline HostConstBuffers::Material CreateDefaultMaterialConstantBuffer()
    {
        HostConstBuffers::Material materialConstants;

        materialConstants.ambient = XMFLOAT3(1.f, 1.f, 1.f);
        materialConstants.diffuse = XMFLOAT3(1.f, 1.f, 1.f);
        materialConstants.specular = XMFLOAT3(1.f, 1.f, 1.f);
        materialConstants.shininess = 0.f;
        materialConstants.alpha = 1.f;

        return materialConstants;
    }

    inline HostConstBuffers::Camera CreateDefaultCameraConstantBuffer()
    {
        HostConstBuffers::Camera buffer;

        buffer.viewMatrix = XMMatrixIdentity();
        buffer.projMatrix = XMMatrixIdentity();
        buffer.invViewMatrix = XMMatrixIdentity();
        buffer.invProjMatrix = XMMatrixIdentity();
        buffer.positionVector = {};

        return buffer;
    }

    inline HostConstBuffers::Lights CreateDefaultLightsConstantBuffer()
    {
        HostConstBuffers::Lights lights;

        lights.viewProj = XMMatrixIdentity();
        lights.viewDir = {0, -1, 0};

        return lights;
    }

    inline HostConstBuffers::SSAO CreateDefaultSSAOConstantBuffer()
    {
        HostConstBuffers::SSAO buffer;

        buffer.kernelSize = 16;
        buffer.kernelRadius = 1.5f;
        buffer.bias = 0.0001f;
        for (int i = 0; i < g_ssaoKernelSize; ++i)
        {
            XMVECTOR const random = RandomNonUniformHalfSphere3();
            float const scale = float(i) / float(g_ssaoKernelSize);
            buffer.kernel[i] = XMVectorScale(random, std::lerp(0.1f, 1.0f, scale * scale));
        }

        return buffer;
    }

    inline HostConstBuffers::Shadows CreateDeafultShadowsConstantBuffer()
    {
        HostConstBuffers::Shadows buffer;

        buffer.bias = 0.01f;
        buffer.pcfEnabled = 1;
        buffer.pcfKernelSize = g_pcfKernelSize;
        buffer.pcfRadius = 2.2f;

        return buffer;
    }

    inline HostConstBuffers::Debug CreateDefaultDebugConstantBuffer()
    {
        HostConstBuffers::Debug buffer;

        buffer.finalOutputIndex = 0;
        buffer.shadowMappingEnabled = 1;
        buffer.normalMappingEnabled = 1;

        return buffer;
    }

    inline HostConstBuffers CreateHostConstBuffers()
    {
        HostConstBuffers buffers;

        buffers.perMaterial.material = CreateDefaultMaterialConstantBuffer();
        buffers.perFrame.camera = CreateDefaultCameraConstantBuffer();
        buffers.infrequent.lights = CreateDefaultLightsConstantBuffer();
        buffers.infrequent.ssao = CreateDefaultSSAOConstantBuffer();
        buffers.infrequent.debug = CreateDefaultDebugConstantBuffer();

        return buffers;
    }

    template <typename T>
    inline ID3D11Buffer *CreateDeviceConstantBuffer(Context const &context)
    {
        ID3D11Buffer *constantBuffer = nullptr;

        D3D11_BUFFER_DESC bufferDescriptor = {};
        ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
        bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
        bufferDescriptor.ByteWidth = sizeof(T);
        bufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDescriptor.CPUAccessFlags = 0;
        bufferDescriptor.MiscFlags = 0;
        bufferDescriptor.StructureByteStride = 0;
        if (FAILED(context.pd3dDevice->CreateBuffer(&bufferDescriptor, nullptr, &constantBuffer)))
        {
            printf("Failed to create const buffer\n");
            return nullptr;
        }

        return constantBuffer;
    }

    inline void CleanupDeviceConstantBuffer(ID3D11Buffer *buffer)
    {
        if (buffer != nullptr)
        {
            buffer->Release();
        }
    }

    inline std::optional<DeviceConstBuffers> CreateDeviceConstantBuffers(Context const &context)
    {
        DeviceConstBuffers cbuffers;

        {
            auto cbuffer = CreateDeviceConstantBuffer<HostConstBuffers::PerInstance>(context);
            if (!cbuffer)
            {
                printf("Failed to create device constant buffer.\n");
                CleanupDeviceConstantBuffers(cbuffers);
                return std::nullopt;
            }
            cbuffers.pPerInstance = cbuffer;
        }

        {
            auto cbuffer = CreateDeviceConstantBuffer<HostConstBuffers::PerMaterial>(context);
            if (!cbuffer)
            {
                printf("Failed to create device constant buffer.\n");
                CleanupDeviceConstantBuffers(cbuffers);
                return std::nullopt;
            }
            cbuffers.pPerMaterial = cbuffer;
        }

        {
            auto cbuffer = CreateDeviceConstantBuffer<HostConstBuffers::PerPass>(context);
            if (!cbuffer)
            {
                printf("Failed to create device constant buffer.\n");
                CleanupDeviceConstantBuffers(cbuffers);
                return std::nullopt;
            }
            cbuffers.pPerPass = cbuffer;
        }

        {
            auto cbuffer = CreateDeviceConstantBuffer<HostConstBuffers::PerFrame>(context);
            if (!cbuffer)
            {
                printf("Failed to create device constant buffer.\n");
                CleanupDeviceConstantBuffers(cbuffers);
                return std::nullopt;
            }
            cbuffers.pPerFrame = cbuffer;
        }

        {
            auto cbuffer = CreateDeviceConstantBuffer<HostConstBuffers::Infrequent>(context);
            if (!cbuffer)
            {
                printf("Failed to create device constant buffer.\n");
                CleanupDeviceConstantBuffers(cbuffers);
                return std::nullopt;
            }
            cbuffers.pInfrequent = cbuffer;
        }

        return cbuffers;
    }

    inline void CleanupDeviceConstantBuffers(DeviceConstBuffers &cbuffers)
    {
        CleanupDeviceConstantBuffer(cbuffers.pPerInstance);
        cbuffers.pPerInstance = nullptr;
        CleanupDeviceConstantBuffer(cbuffers.pPerMaterial);
        cbuffers.pPerMaterial = nullptr;
        CleanupDeviceConstantBuffer(cbuffers.pPerPass);
        cbuffers.pPerPass = nullptr;
        CleanupDeviceConstantBuffer(cbuffers.pPerFrame);
        cbuffers.pPerFrame = nullptr;
        CleanupDeviceConstantBuffer(cbuffers.pInfrequent);
        cbuffers.pInfrequent = nullptr;
    }

    inline void BindConstantBuffers(Context const &context, DeviceConstBuffers const &cbuffers)
    {
        context.pImmediateContext->VSSetConstantBuffers(0, 1, &cbuffers.pPerInstance);
        context.pImmediateContext->VSSetConstantBuffers(1, 1, &cbuffers.pPerMaterial);
        context.pImmediateContext->VSSetConstantBuffers(2, 1, &cbuffers.pPerPass);
        context.pImmediateContext->VSSetConstantBuffers(3, 1, &cbuffers.pPerFrame);
        context.pImmediateContext->VSSetConstantBuffers(4, 1, &cbuffers.pInfrequent);

        context.pImmediateContext->PSSetConstantBuffers(0, 1, &cbuffers.pPerInstance);
        context.pImmediateContext->PSSetConstantBuffers(1, 1, &cbuffers.pPerMaterial);
        context.pImmediateContext->PSSetConstantBuffers(2, 1, &cbuffers.pPerPass);
        context.pImmediateContext->PSSetConstantBuffers(3, 1, &cbuffers.pPerFrame);
        context.pImmediateContext->PSSetConstantBuffers(4, 1, &cbuffers.pInfrequent);

        context.pImmediateContext->CSSetConstantBuffers(0, 1, &cbuffers.pPerInstance);
        context.pImmediateContext->CSSetConstantBuffers(1, 1, &cbuffers.pPerMaterial);
        context.pImmediateContext->CSSetConstantBuffers(2, 1, &cbuffers.pPerPass);
        context.pImmediateContext->CSSetConstantBuffers(3, 1, &cbuffers.pPerFrame);
        context.pImmediateContext->CSSetConstantBuffers(4, 1, &cbuffers.pInfrequent);
    }

    inline void UnbindConstantBuffer(Context const &context)
    {
        ID3D11Buffer *nullBuffer[1] = {nullptr};

        context.pImmediateContext->VSSetConstantBuffers(0, 1, nullBuffer);
        context.pImmediateContext->VSSetConstantBuffers(1, 1, nullBuffer);
        context.pImmediateContext->VSSetConstantBuffers(2, 1, nullBuffer);
        context.pImmediateContext->VSSetConstantBuffers(3, 1, nullBuffer);
        context.pImmediateContext->VSSetConstantBuffers(4, 1, nullBuffer);

        context.pImmediateContext->PSSetConstantBuffers(0, 1, nullBuffer);
        context.pImmediateContext->PSSetConstantBuffers(1, 1, nullBuffer);
        context.pImmediateContext->PSSetConstantBuffers(2, 1, nullBuffer);
        context.pImmediateContext->PSSetConstantBuffers(3, 1, nullBuffer);
        context.pImmediateContext->PSSetConstantBuffers(4, 1, nullBuffer);

        context.pImmediateContext->CSSetConstantBuffers(0, 1, nullBuffer);
        context.pImmediateContext->CSSetConstantBuffers(1, 1, nullBuffer);
        context.pImmediateContext->CSSetConstantBuffers(2, 1, nullBuffer);
        context.pImmediateContext->CSSetConstantBuffers(3, 1, nullBuffer);
        context.pImmediateContext->CSSetConstantBuffers(4, 1, nullBuffer);
    }

} // namespace h2r
