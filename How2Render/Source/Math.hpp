#pragma once

#include <DirectXMath.h>
using namespace DirectX;

namespace h2r::math
{
	inline DirectX::XMMATRIX CreateCameraMatrix(DirectX::XMVECTOR pos, float yaw, float pitch)
	{
		return DirectX::XMMatrixRotationX(pitch)
			* DirectX::XMMatrixRotationY(yaw)
			* DirectX::XMMatrixTranslation(pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2]);
	}

	inline DirectX::XMMATRIX CreateViewMatrix(DirectX::XMVECTOR pos, float yaw, float pitch)
	{
		return DirectX::XMMatrixTranslation(-pos.m128_f32[0], -pos.m128_f32[1], -pos.m128_f32[2])
			* DirectX::XMMatrixRotationY(-yaw)
			* DirectX::XMMatrixRotationX(-pitch);
	}

	inline XMVECTOR CalculateTriangleNormal(XMFLOAT3 const pos[3])
	{
		XMVECTOR v0 = XMLoadFloat3(&pos[0]);
		XMVECTOR v1 = XMLoadFloat3(&pos[1]);
		XMVECTOR v2 = XMLoadFloat3(&pos[2]);
		XMVECTOR n = XMVector3Cross(XMVectorSubtract(v1, v0), XMVectorSubtract(v2, v0));
		return XMVector3Normalize(n);
	}
}
