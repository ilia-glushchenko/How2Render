#pragma once

#include "DirectionalLight.hpp"
#include "Helpers/MeshGenerator.hpp"
#include "Helpers/Random.hpp"
#include "RenderObject.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Texture.hpp"
#include "RenderPass.hpp"

namespace h2r
{

    inline void UpdateInfrequentConstantBuffer(
        Context const& context,
        Application::States const& states,
        DirectionalLight const& lightData,
        DeviceConstBuffers const& cbuffersDevice,
        HostConstBuffers::Infrequent& cbuffersHost);

    inline void UpdatePerFrameConstantBuffer(
        Context const& context,
        Camera const& camera,
        DeviceConstBuffers const& cbuffersDevice,
        HostConstBuffers::PerFrame& cbuffersHost);

    inline void UpdatePerPassConstantBuffer(
        Context const& context,
        Pass const& pass,
        DeviceConstBuffers const& cbuffersDevice,
        HostConstBuffers::PerPass& cbuffersHost);

    inline void DrawOpaqueRenderObjects(
        Context const &context,
        Application::States const &states,
        std::vector<RenderObject> const &objects,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers &cbuffersHost);

    inline void DrawTransparentRenderObjects(
        Context const &context,
        Application::States const &states,
        std::vector<RenderObject> const &objects,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers &cbuffersHost);

    inline void DrawTranslucentRenderObjects(
        Context const &context,
        Application::States const &states,
        std::vector<RenderObject> const &objects,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers &cbuffersHost);

    inline void DrawFullScreen(Context const &context);

} // namespace h2r

namespace h2r
{

    inline void UpdateInfrequentConstantBuffer(
        Context const &context,
        Application::States const& states,
        DirectionalLight const &lightData,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers::Infrequent &cbuffersHost)
    {
        cbuffersHost.ssao.kernelRadius = states.ssaoKernelRadius;
        cbuffersHost.ssao.kernelSize = states.ssaoKernelSize;
        cbuffersHost.ssao.bias = states.ssaoBias;

        cbuffersHost.debug.finalOutputIndex = static_cast<uint32_t>(states.finalOutput);
        cbuffersHost.debug.normalMappingEnabled = static_cast<uint32_t>(states.normalMappingEnabled);
        cbuffersHost.debug.shadowMappingEnabled = static_cast<uint32_t>(states.shadowMappingEnabled);

        cbuffersHost.lights.viewDir = XMVector3Normalize(lightData.direction);
        cbuffersHost.lights.viewProj = lightData.viewProj;

        cbuffersHost.shadows.bias = states.shadowMappingBias;
        cbuffersHost.shadows.pcfEnabled = states.pcfEnabled;
        cbuffersHost.shadows.pcfKernelSize = states.pcfKernelSize;
        cbuffersHost.shadows.pcfRadius = states.pcfRadius;

        context.pImmediateContext->UpdateSubresource(cbuffersDevice.pInfrequent, 0, nullptr, &cbuffersHost, 0, 0);
    }

    inline void UpdatePerFrameConstantBuffer(
        Context const &context,
        Camera const &camera,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers::PerFrame &cbuffersHost)
    {
        cbuffersHost.camera.positionVector = camera.position;
        cbuffersHost.camera.projMatrix = camera.proj;
        cbuffersHost.camera.viewMatrix = camera.view;

        XMVECTOR det;
        cbuffersHost.camera.invViewMatrix = XMMatrixInverse(&det, camera.view);
        cbuffersHost.camera.invProjMatrix = XMMatrixInverse(&det, camera.proj);

        context.pImmediateContext->UpdateSubresource(cbuffersDevice.pPerFrame, 0, nullptr, &cbuffersHost, 0, 0);
    }

    inline void UpdatePerPassConstantBuffer(
        Context const& context,
        Pass const& pass,
        DeviceConstBuffers const& cbuffersDevice,
        HostConstBuffers::PerPass& cbuffersHost)
    {
        cbuffersHost.renderTarget.width = pass.viewportSize.x;
        cbuffersHost.renderTarget.height = pass.viewportSize.y;

        context.pImmediateContext->UpdateSubresource(cbuffersDevice.pPerPass, 0, nullptr, &cbuffersHost, 0, 0);
    }

    inline void UpdatePerMaterialConstantBuffer(
        Context const &context,
        DeviceMesh const &mesh,
        std::vector<DeviceMaterial> const &materials,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers::PerMaterial &cbuffersHost)
    {
        ID3D11ShaderResourceView *shaderResourceViews[MaterialTextureCount] = {};

        if (mesh.materialId != InvalidMaterialId)
        {
            DeviceMaterial const &material = materials[mesh.materialId];

            shaderResourceViews[0] = material.ambientTexture.shaderResourceView;
            shaderResourceViews[1] = material.albedoTexture.shaderResourceView;
            shaderResourceViews[2] = material.specularTexture.shaderResourceView;
            shaderResourceViews[3] = material.normalTexture.shaderResourceView;
            static_assert(MaterialTextureCount == 4);

            cbuffersHost.material.ambient = material.scalarAmbient;
            cbuffersHost.material.diffuse = material.scalarDiffuse;
            cbuffersHost.material.specular = material.scalarSpecular;
            cbuffersHost.material.shininess = material.scalarShininess;
            cbuffersHost.material.alpha = material.scalarAlpha;
            cbuffersHost.material.normalMapAvailabled = material.normalTexture.texture ? 1 : 0;
        }

        context.pImmediateContext->UpdateSubresource(cbuffersDevice.pPerMaterial, 0, nullptr, &cbuffersHost, 0, 0);
        context.pImmediateContext->PSSetShaderResources(0, _countof(shaderResourceViews), shaderResourceViews);
    }

    inline void UpdatePerInstanceConstantBuffer(
        Context const &context,
        DeviceMesh const &mesh,
        Transform const &transform,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers::PerInstance &cbuffersHost)
    {
        cbuffersHost.transform.worldMatrix = transform.world;
        context.pImmediateContext->UpdateSubresource(cbuffersDevice.pPerInstance, 0, nullptr, &cbuffersHost, 0, 0);
    }

    inline void Draw(Context const &context, DeviceMesh const &mesh)
    {
        constexpr uint32_t stride = sizeof(Vertex);
        constexpr uint32_t offset = 0;

        context.pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context.pImmediateContext->IASetVertexBuffers(0, 1, &mesh.vertexBuffer.pVertexBuffer, &stride, &offset);

        if (mesh.indexBuffer.pIndexBuffer)
        {
            context.pImmediateContext->IASetIndexBuffer(mesh.indexBuffer.pIndexBuffer, mesh.indexBuffer.indexFormat, offset);
            context.pImmediateContext->DrawIndexed(mesh.indexBuffer.indexCount, 0, 0);
        }
        else
        {
            context.pImmediateContext->Draw(mesh.vertexBuffer.vertexCount, 0);
        }
    }

    inline void DrawFullScreen(Context const &context)
    {
        static RenderObject const fullscreenTriangleRO = GenerateFullscreenTriangle(context);
        static DeviceMesh const &mesh = fullscreenTriangleRO.model.opaqueMeshes.at(0);
        Draw(context, mesh);
    }

    inline void DrawOpaqueRenderObjects(
        Context const &context,
        Application::States const &states,
        std::vector<RenderObject> const &objects,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers &cbuffersHost)
    {
        if (states.drawOpaque)
        {
            for (auto const &object : objects)
            {
                int32_t currentMaterialId = InvalidMaterialId;

                for (auto const &mesh : object.model.opaqueMeshes)
                {
                    if (currentMaterialId != mesh.materialId)
                    {
                        UpdatePerMaterialConstantBuffer(context, mesh, object.model.materials, cbuffersDevice, cbuffersHost.perMaterial);
                        currentMaterialId = mesh.materialId;
                    }

                    UpdatePerInstanceConstantBuffer(context, mesh, object.transform, cbuffersDevice, cbuffersHost.perInstance);

                    Draw(context, mesh);
                }
            }
        }
    }

    inline void DrawTransparentRenderObjects(
        Context const &context,
        Application::States const &states,
        std::vector<RenderObject> const &objects,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers &cbuffersHost)
    {
        if (states.drawTransparent)
        {
            for (auto const &object : objects)
            {
                int32_t currentMaterialId = InvalidMaterialId;

                for (auto const &mesh : object.model.transparentMeshes)
                {
                    if (currentMaterialId != mesh.materialId)
                    {
                        UpdatePerMaterialConstantBuffer(context, mesh, object.model.materials, cbuffersDevice, cbuffersHost.perMaterial);
                        currentMaterialId = mesh.materialId;
                    }

                    UpdatePerInstanceConstantBuffer(context, mesh, object.transform, cbuffersDevice, cbuffersHost.perInstance);

                    Draw(context, mesh);
                }
            }
        }
    }

    inline void DrawTranslucentRenderObjects(
        Context const &context,
        Application::States const &states,
        std::vector<RenderObject> const &objects,
        DeviceConstBuffers const &cbuffersDevice,
        HostConstBuffers &cbuffersHost)
    {
        if (states.drawTranslucent)
        {
            for (auto const &object : objects)
            {
                int32_t currentMaterialId = InvalidMaterialId;

                for (auto const &mesh : object.model.transparentMeshes)
                {
                    if (currentMaterialId != mesh.materialId)
                    {
                        UpdatePerMaterialConstantBuffer(context, mesh, object.model.materials, cbuffersDevice, cbuffersHost.perMaterial);
                        currentMaterialId = mesh.materialId;
                    }

                    UpdatePerInstanceConstantBuffer(context, mesh, object.transform, cbuffersDevice, cbuffersHost.perInstance);

                    Draw(context, mesh);
                }
            }
        }
    }

} // namespace h2r
