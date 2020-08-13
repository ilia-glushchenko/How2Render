#pragma once

#include "Material.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/IndexBuffer.hpp"
#include "Wrapper/VertexBuffer.hpp"
#include <vector>

namespace h2r
{

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT2 textureCoordinate;
	};

	struct HostMesh
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	struct DeviceMesh
	{
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
		int materialId = InvalidMaterialId;
		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	};

	inline DeviceMesh CreateDeviceMesh(Context const &context, HostMesh const &hostMesh,
									   int materialId = InvalidMaterialId)
	{
		DeviceMesh mesh;

		mesh.vertexBuffer = CreateVertexBuffer(context, hostMesh.vertices);
		if (!hostMesh.indices.empty())
		{
			mesh.indexBuffer = CreateIndexBuffer(context, hostMesh.indices);
		}
		mesh.materialId = materialId;

		return mesh;
	}

	inline void CleanupDeviceMesh(DeviceMesh &mesh)
	{
		ReleaseVertexBuffer(mesh.vertexBuffer);
		ReleaseIndexBuffer(mesh.indexBuffer);
	}

} // namespace h2r
