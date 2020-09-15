#pragma once

#include "Math.hpp"
#include "Wrapper/Context.hpp"
#include <cstdint>
#include <d3d11.h>
#include <vector>

namespace h2r
{

	struct VertexBuffer
	{
		ID3D11Buffer *pVertexBuffer = nullptr;
		uint32_t vertexCount = 0;
	};

	template <typename VertexType>
	VertexBuffer CreateVertexBuffer(Context const &context, std::vector<VertexType> const &vertices)
	{
		D3D11_BUFFER_DESC bufferDesc;

		bufferDesc.ByteWidth = (UINT)(sizeof(VertexType) * vertices.size());
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data = {vertices.data(), 0, 0};
		VertexBuffer buffer;

		auto hr = context.pd3dDevice->CreateBuffer(&bufferDesc, &data, &buffer.pVertexBuffer);
		assert(SUCCEEDED(hr));
		buffer.vertexCount = (uint32_t)vertices.size();

		return buffer;
	}

	inline void CleanupVertexBuffer(VertexBuffer &buffer)
	{
		if (buffer.pVertexBuffer != nullptr)
		{
			buffer.pVertexBuffer->Release();
			buffer.pVertexBuffer = nullptr;
		}

		buffer.vertexCount = 0;
	}

} // namespace h2r
