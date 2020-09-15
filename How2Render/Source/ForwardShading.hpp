#pragma once

#include "Application.hpp"
#include "Camera.hpp"
#include "Helpers/MeshGenerator.hpp"
#include "RenderObject.hpp"
#include "Window.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/Shader.hpp"
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

	inline void ShadeForward(
		Context const &context,
		ForwardShaders const &shaders,
		Application::States const &states,
		std::vector<RenderObject> const &objects)
	{
		// Draw opaque
		if (states.drawOpaque)
		{
			BindBlendState(context, shaders.blendStates.none);
			BindShaders(context, shaders.shadingPass);
			BindConstantBuffers(context, shaders.cbuffers);
			BindSamplers(context, shaders.samplers);

			for (auto const &object : objects)
			{
				for (auto const &mesh : object.model.opaqueMeshes)
				{
					UpdatePerMeshConstantBuffer(
						context,
						shaders.cbuffers,
						object.model.materials,
						mesh,
						object.transform);

					Draw(context, mesh);
				}
			}
		}

		// Draw transparent
		if (states.drawTransparent)
		{
			BindBlendState(context, shaders.blendStates.alphaToCoverage);
			BindShaders(context, shaders.shadingPass);
			BindConstantBuffers(context, shaders.cbuffers);
			BindSamplers(context, shaders.samplers);

			for (auto const &object : objects)
			{
				for (auto const &mesh : object.model.transparentMeshes)
				{
					UpdatePerMeshConstantBuffer(
						context,
						shaders.cbuffers,
						object.model.materials,
						mesh,
						object.transform);

					Draw(context, mesh);
				}
			}
		}
	}

} // namespace h2r