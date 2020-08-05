#pragma once

struct IndexBuffer
{
	ID3D11Buffer *pIndexBuffer = nullptr;
	uint32_t indexCount = 0;
	DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
};

template<typename IndexType>
IndexBuffer CreateIndexBuffer(Context const& context, std::vector<IndexType> const& indices)
{
	D3D11_BUFFER_DESC bufferDesc;

	bufferDesc.ByteWidth = (UINT)(sizeof(IndexType) * indices.size());
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data = {indices.data(), 0, 0};
	IndexBuffer buffer;

	auto hr = context.pd3dDevice->CreateBuffer(&bufferDesc, &data, &buffer.pIndexBuffer);
	assert(SUCCEEDED(hr));
	buffer.indexCount = (uint32_t)indices.size();

	if (sizeof(IndexType) == sizeof(uint16_t))
		buffer.indexFormat = DXGI_FORMAT_R16_UINT;
	else
		buffer.indexFormat = DXGI_FORMAT_R32_UINT;

	return buffer;
}

void ReleaseIndexBuffer(IndexBuffer& buffer)
{
	if (buffer.pIndexBuffer != nullptr)
	{
		buffer.pIndexBuffer->Release();
		buffer.pIndexBuffer = nullptr;
	}

	buffer.indexCount = 0;
}
