#pragma once

#include "Mesh.hpp"
#include "Material.hpp"

namespace h2r
{
	struct DeviceModel
	{
		std::vector<DeviceMesh> meshes;
		std::vector<DeviceMaterial> materials;
	};

	void DrawModel(Context const& context, Shaders const& shaders, DeviceModel const& model)
	{
		context.pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context.pImmediateContext->VSSetShader(shaders.pVertexShader, nullptr, 0);
		context.pImmediateContext->VSSetConstantBuffers(0, 1, &shaders.pConstantBuffer);

		context.pImmediateContext->PSSetShader(shaders.pPixelShader, nullptr, 0);
		context.pImmediateContext->PSSetConstantBuffers(0, 1, &shaders.pConstantBuffer);
		context.pImmediateContext->PSSetConstantBuffers(1, 1, &shaders.pMaterialConstants);
		context.pImmediateContext->PSSetSamplers(0, 1, &shaders.pTrilinearSampler);

		for (auto const& mesh : model.meshes)
		{
			ID3D11ShaderResourceView *shaderResourceViews[3];
			MaterialConstantBuffer materialConstants;

			if (mesh.materialId > InvalidMaterialId)
			{
				DeviceMaterial const& material = model.materials[mesh.materialId];

				shaderResourceViews[0] = material.ambientTexture.shaderResourceView;
				shaderResourceViews[1] = material.albedoTexture.shaderResourceView;
				shaderResourceViews[2] = material.specularTexture.shaderResourceView;

				materialConstants.ambient = material.ambient;
				materialConstants.diffuse = material.diffuse;
				materialConstants.specular = material.specular;
				materialConstants.shininess = material.shininess;
			}
			else
			{
				for (int i = 0; i < _countof(shaderResourceViews); ++i)
					shaderResourceViews[i] = nullptr;

				materialConstants.ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);
				materialConstants.diffuse = XMFLOAT3(1.f, 1.f, 1.f);
				materialConstants.specular = XMFLOAT3(1.f, 1.f, 1.f);
				materialConstants.shininess = 0.f;
			}

			// Update material properties
			context.pImmediateContext->UpdateSubresource(
				shaders.pMaterialConstants, 0, nullptr, &materialConstants, 0, 0);
			// Set material textures
			context.pImmediateContext->PSSetShaderResources(0, _countof(shaderResourceViews), shaderResourceViews);

			constexpr uint32_t stride = sizeof(Vertex);
			constexpr uint32_t offset = 0;

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
	}

	void FreeModel(DeviceModel& model)
	{
		for (auto& mesh : model.meshes)
		{
			ReleaseVertexBuffer(mesh.vertexBuffer);
			if (mesh.indexBuffer.pIndexBuffer)
				ReleaseIndexBuffer(mesh.indexBuffer);
		}

		model.meshes.clear();
	}
} // namespace h2r
