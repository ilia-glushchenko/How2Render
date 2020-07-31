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
}
