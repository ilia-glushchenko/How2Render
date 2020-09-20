#pragma once

#include "Helpers/MeshGenerator.hpp"
#include "RenderObject.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Texture.hpp"

namespace h2r
{

	inline void UpdatePerMeshConstantBuffer(
		Context const &context,
		DeviceConstBuffers const &cbuffers,
		std::vector<DeviceMaterial> const &materials,
		DeviceMesh const &mesh,
		Transform const &transform)
	{
		// Update Material Constant Buffer
		{
			ID3D11ShaderResourceView *shaderResourceViews[3] = {};
			auto materialConstants = CreateDefaultMaterialConstantBuffer();

			if (mesh.materialId != InvalidMaterialId)
			{
				DeviceMaterial const &material = materials[mesh.materialId];

				shaderResourceViews[0] = material.ambientTexture.shaderResourceView;
				shaderResourceViews[1] = material.albedoTexture.shaderResourceView;
				shaderResourceViews[2] = material.specularTexture.shaderResourceView;

				materialConstants.ambient = material.scalarAmbient;
				materialConstants.diffuse = material.scalarDiffuse;
				materialConstants.specular = material.scalarSpecular;
				materialConstants.shininess = material.scalarShininess;
				materialConstants.alpha = material.scalarAlpha;
			}

			context.pImmediateContext->UpdateSubresource(cbuffers.pMaterialConstants, 0, nullptr, &materialConstants, 0, 0);
			context.pImmediateContext->PSSetShaderResources(0, _countof(shaderResourceViews), shaderResourceViews);
		}

		// Update Transform Constant Buffer
		{
			TransformHostConstBuffer transformConstants;
			transformConstants.worldMatrix = transform.world;

			context.pImmediateContext->UpdateSubresource(cbuffers.pTranformConstants, 0, nullptr, &transformConstants, 0, 0);
		}
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

	inline void DrawFullScreen(
		Context const &context,
		ShaderProgram const &shaderProgram,
		BlendState const &blendState,
		TextureSamplers const &samplers,
		DeviceConstBuffers const &cbuffers,
		uint32_t sourceCount,
		ID3D11ShaderResourceView *const *resources)
	{
		static RenderObject const fullscreenTriangleRO = GenerateFullscreenTriangle(context);
		static DeviceMesh const &mesh = fullscreenTriangleRO.model.opaqueMeshes.at(0);

		constexpr uint32_t stride = sizeof(Vertex);
		constexpr uint32_t offset = 0;

		BindBlendState(context, blendState);
		BindShaders(context, shaderProgram);
		BindSamplers(context, samplers);
		BindConstantBuffers(context, cbuffers);
		context.pImmediateContext->PSSetShaderResources(0, sourceCount, resources);

		context.pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context.pImmediateContext->IASetVertexBuffers(0, 1, &mesh.vertexBuffer.pVertexBuffer, &stride, &offset);
		context.pImmediateContext->IASetIndexBuffer(mesh.indexBuffer.pIndexBuffer, mesh.indexBuffer.indexFormat, offset);

		context.pImmediateContext->DrawIndexed(mesh.indexBuffer.indexCount, 0, 0);
	}

} // namespace h2r
