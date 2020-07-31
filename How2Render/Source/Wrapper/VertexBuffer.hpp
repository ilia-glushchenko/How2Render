#pragma once

#include "Wrapper/Context.hpp"
#include "Math.hpp"
#include <d3d11.h>

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 ui;
};

struct VertexBuffer
{
	ID3D11Buffer *pVertexBuffer;
	uint32_t vertexCount;
};

inline VertexBuffer CreateVertexBuffer(Context const &context)
{
	VertexBuffer buffer = {nullptr, 3};

	static const Vertex vertices[3] = {
		Vertex{
			DirectX::XMFLOAT3{-1.0, 3.0, 0.5},
			DirectX::XMFLOAT2{0.0, -1.0},
		},
		Vertex{
			DirectX::XMFLOAT3{3.0, -1.0, 0.5},
			DirectX::XMFLOAT2{2.0, 1.0},
		},
		Vertex{
			DirectX::XMFLOAT3{-1.0, -1.0, 0.5},
			DirectX::XMFLOAT2{0.0, 1.0},
		},
	};

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * 3;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	auto hr = context.pd3dDevice->CreateBuffer(&bd, &InitData, &buffer.pVertexBuffer);
	assert(SUCCEEDED(hr));

	// Set vertex buffer
	uint32_t stride = sizeof(Vertex);
	uint32_t offset = 0;
	context.pImmediateContext->IASetVertexBuffers(0, 1, &buffer.pVertexBuffer, &stride, &offset);

	// Set primitive topology
	context.pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return buffer;
}

void CleanupBuffer(VertexBuffer const &buffer)
{
	if (buffer.pVertexBuffer != nullptr)
	{
		buffer.pVertexBuffer->Release();
	}
};