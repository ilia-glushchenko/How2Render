#pragma once

#include "Wrapper/VertexBuffer.hpp"
#include "Wrapper/IndexBuffer.hpp"

#include <GeometricPrimitive.h>

struct Mesh
{
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

Mesh GenerateSphere(Context const& context, float radius, uint32_t tesselation)
{
	std::vector<DirectX::GeometricPrimitive::VertexType> vertices;
	std::vector<uint16_t> indices;

	DirectX::GeometricPrimitive::CreateSphere(vertices, indices, radius * 2.f, tesselation, false);

    Mesh sphere;

    sphere.vertexBuffer = CreateVertexBuffer(context, vertices);
    sphere.indexBuffer = CreateIndexBuffer(context, indices);

	return sphere;
}

void CleanupMesh(Mesh& mesh)
{
    ReleaseVertexBuffer(mesh.vertexBuffer);
    ReleaseIndexBuffer(mesh.indexBuffer);
}
