#pragma once

#include "Helpers/MeshGenerator.hpp"
#include "Helpers/Random.hpp"
#include "RenderObject.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Texture.hpp"

namespace h2r
{

    inline void UpdateInfrequentConstantBuffers(
        Context const &context,
        Application::States const &states,
        DeviceConstBuffers const &deviceBuffers,
        HostConstBuffers &hostBuffers);

    inline void UpdatePerFrameConstantBuffers(
        Context const &context,
        Camera const &camera,
        DeviceConstBuffers const &deviceBuffers,
        HostConstBuffers &hostBuffers);

    inline void UpdatePerMaterialConstantBuffer(
        Context const &context,
        DeviceMesh const &mesh,
        std::vector<DeviceMaterial> const &materials,
        DeviceConstBuffers const &deviceBuffers,
        HostConstBuffers &hostBuffers);

    inline void UpdatePerInstanceConstantBuffer(
        Context const &context,
        Transform const &transform,
        DeviceConstBuffers const &deviceBuffers,
        HostConstBuffers &hostBuffers);

    inline void DrawFullScreen(Context const &context);

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

} // namespace h2r

namespace h2r
{

    inline void UpdateInfrequentConstantBuffers(
        Context const &context,
        Application::States const &states,
        DeviceConstBuffers const &deviceBuffers,
        HostConstBuffers &hostBuffers)
    {
        hostBuffers.infrequent.finalOutputIndex = static_cast<uint32_t>(states.finalOutput);
        hostBuffers.infrequent.ssaoKernelRadius = states.ssaoKernelRadius;
        hostBuffers.infrequent.ssaoKernelSize = states.ssaoKernelSize;
        hostBuffers.infrequent.ssaoBias = states.ssaoBias;
        hostBuffers.infrequent.normalMappingEnabled = static_cast<uint32_t>(states.normalMappingEnabled);

        context.pImmediateContext->UpdateSubresource(deviceBuffers.pInfrequentCB, 0, nullptr, &hostBuffers.infrequent, 0, 0);
    }

    inline void UpdatePerFrameConstantBuffers(
        Context const &context,
        Camera const &camera,
        DeviceConstBuffers const &deviceBuffers,
        HostConstBuffers &hostBuffers)
    {
        hostBuffers.perFrame.positionVector = camera.position;
        hostBuffers.perFrame.projMatrix = camera.proj;
        hostBuffers.perFrame.viewMatrix = camera.view;

        XMVECTOR det;
        hostBuffers.perFrame.invViewMatrix = XMMatrixInverse(&det, camera.view);
        hostBuffers.perFrame.invProjMatrix = XMMatrixInverse(&det, camera.proj);

        context.pImmediateContext->UpdateSubresource(deviceBuffers.pPerFrameCB, 0, nullptr, &hostBuffers.perFrame, 0, 0);
    }

    inline void UpdatePerMaterialConstantBuffer(
        Context const &context,
        DeviceMesh const &mesh,
        std::vector<DeviceMaterial> const &materials,
        DeviceConstBuffers const &deviceBuffers,
        HostConstBuffers &hostBuffers)
    {
        ID3D11ShaderResourceView *shaderResourceViews[4] = {};

        if (mesh.materialId != InvalidMaterialId)
        {
            DeviceMaterial const &material = materials[mesh.materialId];

            shaderResourceViews[0] = material.ambientTexture.shaderResourceView;
            shaderResourceViews[1] = material.albedoTexture.shaderResourceView;
            shaderResourceViews[2] = material.specularTexture.shaderResourceView;
            shaderResourceViews[3] = material.normalTexture.shaderResourceView;

            hostBuffers.perMaterial.ambient = material.scalarAmbient;
            hostBuffers.perMaterial.diffuse = material.scalarDiffuse;
            hostBuffers.perMaterial.specular = material.scalarSpecular;
            hostBuffers.perMaterial.shininess = material.scalarShininess;
            hostBuffers.perMaterial.alpha = material.scalarAlpha;
        }

        context.pImmediateContext->UpdateSubresource(deviceBuffers.pPerMaterialCB, 0, nullptr, &hostBuffers.perMaterial, 0, 0);
        context.pImmediateContext->PSSetShaderResources(0, _countof(shaderResourceViews), shaderResourceViews);
    }

    inline void UpdatePerInstanceConstantBuffer(
        Context const &context,
        Transform const &transform,
        DeviceConstBuffers const &deviceBuffers,
        HostConstBuffers &hostBuffers)
    {
        hostBuffers.perInstance.worldMatrix = transform.world;
        context.pImmediateContext->UpdateSubresource(deviceBuffers.pPerInstanceCB, 0, nullptr, &hostBuffers.perInstance, 0, 0);
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
                for (auto const &mesh : object.model.opaqueMeshes)
                {
                    UpdatePerInstanceConstantBuffer(
                        context, object.transform, cbuffersDevice, cbuffersHost);

                    UpdatePerMaterialConstantBuffer(
                        context, mesh, object.model.materials, cbuffersDevice, cbuffersHost);

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
                for (auto const &mesh : object.model.transparentMeshes)
                {
                    UpdatePerInstanceConstantBuffer(
                        context, object.transform, cbuffersDevice, cbuffersHost);

                    UpdatePerMaterialConstantBuffer(
                        context, mesh, object.model.materials, cbuffersDevice, cbuffersHost);

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
                for (auto const &mesh : object.model.transparentMeshes)
                {
                    UpdatePerInstanceConstantBuffer(
                        context, object.transform, cbuffersDevice, cbuffersHost);

                    UpdatePerMaterialConstantBuffer(
                        context, mesh, object.model.materials, cbuffersDevice, cbuffersHost);

                    Draw(context, mesh);
                }
            }
        }
    }

} // namespace h2r
