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

	void DrawModel(DeviceModel const& model, Context const& context, Shaders const& shaders, BlendStates const& blendStates)
	{
		context.pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (auto const& mesh : model.meshes)
		{
			constexpr uint32_t stride = sizeof(Vertex);
			constexpr uint32_t offset = 0;

			context.pImmediateContext->IASetVertexBuffers(0, 1, &mesh.vertexBuffer.pVertexBuffer, &stride, &offset);

			if (mesh.vertexRanges.empty())
			{
				if (mesh.materialId > InvalidMaterialId)
				{
					SetMaterial(model.materials[mesh.materialId], context, shaders, blendStates);
				}
				else
				{
					SetNullMaterial(context, shaders);
				}

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
			else
			{
				for (auto const& range : mesh.vertexRanges)
				{
					if (range.materialId > InvalidMaterialId)
					{
						SetMaterial(model.materials[range.materialId], context, shaders, blendStates);
					}
					else
					{
						SetNullMaterial(context, shaders);
					}

					context.pImmediateContext->Draw(range.vertexCount, range.firstVertex);
				}
			}
		}

		context.pImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
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
