#pragma once

#include "Math.hpp"
#include "Random.hpp"
#include "Wrapper/Context.hpp"
#include <cassert>

namespace h2r
{

    constexpr uint32_t g_ssaoKernelSize = 64;

    struct HostConstBuffers
    {
        struct Infrequent
        {
            uint32_t finalOutputIndex;
            uint32_t ssaoKernelSize;
            float ssaoKernelRadius;
            float ssaoBias;
            XMVECTOR ssaoKernel[g_ssaoKernelSize];
            uint32_t normalMappingEnabled;
            XMFLOAT3 padd0;
        };

        struct PerFrame
        {
            XMMATRIX viewMatrix;
            XMMATRIX projMatrix;
            XMMATRIX invViewMatrix;
            XMMATRIX invProjMatrix;
            XMVECTOR positionVector;
        };

        struct PerMaterial
        {
            XMFLOAT3 ambient;
            float padd0;
            XMFLOAT3 diffuse;
            float padd1;
            XMFLOAT3 specular;
            float shininess;

            float alpha;
            XMFLOAT3 padd2;
        };

        struct PerInstance
        {
            XMMATRIX worldMatrix;
        };

        PerInstance perInstance;
        PerMaterial perMaterial;
        PerFrame perFrame;
        Infrequent infrequent;
    };

    struct DeviceConstBuffers
    {
        ID3D11Buffer *pPerInstanceCB = nullptr;
        ID3D11Buffer *pPerMaterialCB = nullptr;
        ID3D11Buffer *pPerFrameCB = nullptr;
        ID3D11Buffer *pInfrequentCB = nullptr;
    };

    inline HostConstBuffers CreateHostConstBuffers();

    inline DeviceConstBuffers CreateDeviceConstantBuffers(Context const &context);

    inline void CleanupDeviceConstantBuffers(DeviceConstBuffers &cbuffers);

    inline void BindConstantBuffers(Context const &context, DeviceConstBuffers const &cbuffers);

    inline void UnbindConstantBuffer(Context const &context);

} // namespace h2r

namespace h2r
{

    inline void BindConstantBuffers(
        Context const &context,
        DeviceConstBuffers const &cbuffers)
    {
        context.pImmediateContext->VSSetConstantBuffers(0, 1, &cbuffers.pPerInstanceCB);
        context.pImmediateContext->VSSetConstantBuffers(1, 1, &cbuffers.pPerMaterialCB);
        context.pImmediateContext->VSSetConstantBuffers(2, 1, &cbuffers.pPerFrameCB);
        context.pImmediateContext->VSSetConstantBuffers(3, 1, &cbuffers.pInfrequentCB);

        context.pImmediateContext->PSSetConstantBuffers(0, 1, &cbuffers.pPerInstanceCB);
        context.pImmediateContext->PSSetConstantBuffers(1, 1, &cbuffers.pPerMaterialCB);
        context.pImmediateContext->PSSetConstantBuffers(2, 1, &cbuffers.pPerFrameCB);
        context.pImmediateContext->PSSetConstantBuffers(3, 1, &cbuffers.pInfrequentCB);

        context.pImmediateContext->CSSetConstantBuffers(0, 1, &cbuffers.pPerInstanceCB);
        context.pImmediateContext->CSSetConstantBuffers(1, 1, &cbuffers.pPerMaterialCB);
        context.pImmediateContext->CSSetConstantBuffers(2, 1, &cbuffers.pPerFrameCB);
        context.pImmediateContext->CSSetConstantBuffers(3, 1, &cbuffers.pInfrequentCB);
    }

    inline void UnbindConstantBuffer(Context const &context)
    {
        ID3D11Buffer *nullBuffer[1] = {nullptr};

        context.pImmediateContext->VSSetConstantBuffers(0, 1, nullBuffer);
        context.pImmediateContext->VSSetConstantBuffers(1, 1, nullBuffer);
        context.pImmediateContext->VSSetConstantBuffers(2, 1, nullBuffer);
        context.pImmediateContext->VSSetConstantBuffers(3, 1, nullBuffer);

        context.pImmediateContext->PSSetConstantBuffers(0, 1, nullBuffer);
        context.pImmediateContext->PSSetConstantBuffers(1, 1, nullBuffer);
        context.pImmediateContext->PSSetConstantBuffers(2, 1, nullBuffer);
        context.pImmediateContext->PSSetConstantBuffers(3, 1, nullBuffer);

        context.pImmediateContext->CSSetConstantBuffers(0, 1, nullBuffer);
        context.pImmediateContext->CSSetConstantBuffers(1, 1, nullBuffer);
        context.pImmediateContext->CSSetConstantBuffers(2, 1, nullBuffer);
        context.pImmediateContext->CSSetConstantBuffers(3, 1, nullBuffer);
    }

    inline HostConstBuffers::PerMaterial CreatePerMaterialHostConstBuffer()
    {
        HostConstBuffers::PerMaterial materialConstants;
        materialConstants.ambient = XMFLOAT3(1.f, 1.f, 1.f);
        materialConstants.diffuse = XMFLOAT3(1.f, 1.f, 1.f);
        materialConstants.specular = XMFLOAT3(1.f, 1.f, 1.f);
        materialConstants.shininess = 0.f;
        materialConstants.alpha = 1.f;
        return materialConstants;
    }

    inline HostConstBuffers::PerFrame CreatePerFrameConstBuffer()
    {
        HostConstBuffers::PerFrame buffer;

        buffer.viewMatrix = XMMatrixIdentity();
        buffer.projMatrix = XMMatrixIdentity();
        buffer.invViewMatrix = XMMatrixIdentity();
        buffer.invProjMatrix = XMMatrixIdentity();
        buffer.positionVector = {};

        return buffer;
    }

    inline HostConstBuffers::Infrequent CreateInfrequentConstBuffer()
    {
        HostConstBuffers::Infrequent buffer;

        buffer.finalOutputIndex = 0;
        buffer.ssaoKernelSize = 16;
        buffer.ssaoKernelRadius = 1.5f;
        buffer.ssaoBias = 0.0001f;
        buffer.normalMappingEnabled = 1;

        for (int i = 0; i < g_ssaoKernelSize; ++i)
        {
            XMVECTOR const random = RandomNonUniformHalfSphere3();
            float const scale = float(i) / float(g_ssaoKernelSize);
            buffer.ssaoKernel[i] = XMVectorScale(random, std::lerp(0.1f, 1.0f, scale * scale));
        }

        return buffer;
    }

    template <typename T>
    ID3D11Buffer *CreateDeviceConstantBuffer(Context const &context)
    {
        ID3D11Buffer *constantBuffer = nullptr;

        D3D11_BUFFER_DESC bufferDescriptor = {};
        bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
        bufferDescriptor.ByteWidth = sizeof(T);
        bufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDescriptor.CPUAccessFlags = 0;
        auto const hr = context.pd3dDevice->CreateBuffer(&bufferDescriptor, nullptr, &constantBuffer);
        assert(SUCCEEDED(hr));

        return constantBuffer;
    }

    inline void CleanupDeviceConstantBuffer(ID3D11Buffer *buffer)
    {
        if (buffer != nullptr)
        {
            buffer->Release();
        }
    }

    inline HostConstBuffers CreateHostConstBuffers()
    {
        HostConstBuffers buffers;

        buffers.perMaterial = CreatePerMaterialHostConstBuffer();
        buffers.perFrame = CreatePerFrameConstBuffer();
        buffers.infrequent = CreateInfrequentConstBuffer();

        return buffers;
    }

    inline DeviceConstBuffers CreateDeviceConstantBuffers(Context const &context)
    {
        DeviceConstBuffers cbuffers;

        cbuffers.pPerInstanceCB = CreateDeviceConstantBuffer<HostConstBuffers::PerInstance>(context);
        cbuffers.pPerMaterialCB = CreateDeviceConstantBuffer<HostConstBuffers::PerMaterial>(context);
        cbuffers.pPerFrameCB = CreateDeviceConstantBuffer<HostConstBuffers::PerFrame>(context);
        cbuffers.pInfrequentCB = CreateDeviceConstantBuffer<HostConstBuffers::Infrequent>(context);

        return cbuffers;
    }

    inline void CleanupDeviceConstantBuffers(DeviceConstBuffers &cbuffers)
    {
        CleanupDeviceConstantBuffer(cbuffers.pPerInstanceCB);
        cbuffers.pPerInstanceCB = nullptr;
        CleanupDeviceConstantBuffer(cbuffers.pPerMaterialCB);
        cbuffers.pPerMaterialCB = nullptr;
        CleanupDeviceConstantBuffer(cbuffers.pPerFrameCB);
        cbuffers.pPerFrameCB = nullptr;
        CleanupDeviceConstantBuffer(cbuffers.pInfrequentCB);
        cbuffers.pInfrequentCB = nullptr;
    }

} // namespace h2r
