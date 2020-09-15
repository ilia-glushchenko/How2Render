#pragma once

#include "Math.hpp"
#include "Wrapper/Context.hpp"
#include <cassert>

namespace h2r
{

	struct CameraHostConstBuffer
	{
		XMMATRIX viewMatrix;
		XMMATRIX projMatrix;
		XMVECTOR positionVector;
	};

	struct TransformHostConstBuffer
	{
		XMMATRIX worldMatrix;
	};

	struct MaterialHostConstBuffer
	{
		XMFLOAT3 ambient;
		float padd0;
		XMFLOAT3 diffuse;
		float padd1;
		XMFLOAT3 specular;
		float shininess;
		float alpha;
		XMFLOAT3 padd2;
	};

	struct DeviceConstBuffers
	{
		ID3D11Buffer *pCameraConstants = nullptr;
		ID3D11Buffer *pTranformConstants = nullptr;
		ID3D11Buffer *pMaterialConstants = nullptr;
	};

	inline void BindConstantBuffers(
		Context const &context,
		DeviceConstBuffers const &cbuffers)
	{
		context.pImmediateContext->VSSetConstantBuffers(0, 1, &cbuffers.pCameraConstants);
		context.pImmediateContext->VSSetConstantBuffers(1, 1, &cbuffers.pTranformConstants);
		context.pImmediateContext->VSSetConstantBuffers(2, 1, &cbuffers.pMaterialConstants);

		context.pImmediateContext->PSSetConstantBuffers(0, 1, &cbuffers.pCameraConstants);
		context.pImmediateContext->PSSetConstantBuffers(1, 1, &cbuffers.pTranformConstants);
		context.pImmediateContext->PSSetConstantBuffers(2, 1, &cbuffers.pMaterialConstants);
	}

	inline MaterialHostConstBuffer CreateDefaultMaterialConstantBuffer()
	{
		MaterialHostConstBuffer materialConstants;
		materialConstants.ambient = XMFLOAT3(1.f, 1.f, 1.f);
		materialConstants.diffuse = XMFLOAT3(1.f, 1.f, 1.f);
		materialConstants.specular = XMFLOAT3(1.f, 1.f, 1.f);
		materialConstants.shininess = 0.f;
		materialConstants.alpha = 1.f;
		return materialConstants;
	}

	template <typename T>
	ID3D11Buffer *CreateDeviceConstantBuffer(Context const &context)
	{
		ID3D11Buffer *constantBuffer = nullptr;

		D3D11_BUFFER_DESC bufferDescriptor = {};
		bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
		bufferDescriptor.ByteWidth = sizeof(T);
		bufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescriptor.CPUAccessFlags = 0;
		auto const hr = context.pd3dDevice->CreateBuffer(&bufferDescriptor, nullptr, &constantBuffer);
		assert(SUCCEEDED(hr));

		return constantBuffer;
	}

	inline void CleanupDeviceConstantBuffer(ID3D11Buffer *buffer)
	{
		if (buffer != nullptr)
		{
			buffer->Release();
		}
	}

	inline DeviceConstBuffers CreateDeviceConstantBuffers(Context const &context)
	{
		DeviceConstBuffers cbuffers;

		cbuffers.pCameraConstants = CreateDeviceConstantBuffer<CameraHostConstBuffer>(context);
		cbuffers.pMaterialConstants = CreateDeviceConstantBuffer<MaterialHostConstBuffer>(context);
		cbuffers.pTranformConstants = CreateDeviceConstantBuffer<TransformHostConstBuffer>(context);

		return cbuffers;
	}

	inline void CleanupDeviceConstantBuffers(DeviceConstBuffers &cbuffers)
	{
		CleanupDeviceConstantBuffer(cbuffers.pCameraConstants);
		cbuffers.pCameraConstants = nullptr;
		CleanupDeviceConstantBuffer(cbuffers.pMaterialConstants);
		cbuffers.pMaterialConstants = nullptr;
		CleanupDeviceConstantBuffer(cbuffers.pTranformConstants);
		cbuffers.pTranformConstants = nullptr;
	}

} // namespace h2r
