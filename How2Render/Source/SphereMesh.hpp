#pragma once
#include "Wrapper/Context.hpp"
#include <GeometricPrimitive.h>

struct Mesh
{
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	uint32_t numIndices = 0;
	DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

Mesh GenerateSphere(Context const& context, float radius, uint32_t tesselation)
{
	std::vector<DirectX::GeometricPrimitive::VertexType> vertices;
	std::vector<uint16_t> indices;

	DirectX::GeometricPrimitive::CreateSphere(vertices, indices, radius * 2.f, tesselation, false);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = (UINT)(sizeof(DirectX::GeometricPrimitive::VertexType) * vertices.size());
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA data = {vertices.data(), 0, 0};

	Mesh sphere;

	auto hr = context.pd3dDevice->CreateBuffer(&bufferDesc, &data, &sphere.vertexBuffer);
	assert(SUCCEEDED(hr));

	bufferDesc.ByteWidth = (UINT)(sizeof(uint16_t) * indices.size());
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	data.pSysMem = indices.data();

	hr = context.pd3dDevice->CreateBuffer(&bufferDesc, &data, &sphere.indexBuffer);
	assert(SUCCEEDED(hr));

	sphere.numIndices = (uint32_t)indices.size();
	return sphere;
}

void CleanupMesh(Mesh& mesh)
{
	if (mesh.vertexBuffer != nullptr)
	{
		mesh.vertexBuffer->Release();
		mesh.vertexBuffer = nullptr;
	}
	if (mesh.indexBuffer != nullptr)
	{
		mesh.indexBuffer->Release();
		mesh.indexBuffer = nullptr;
	}
	mesh.numIndices = 0;
}
