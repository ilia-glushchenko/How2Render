#pragma once

#include <DirectXMath.h>
using namespace DirectX;

namespace math
{
	inline XMMATRIX CreateCameraMatrix(XMVECTOR pos, float yaw, float pitch)
	{
		return XMMatrixRotationX(pitch)
			* XMMatrixRotationY(yaw)
			* XMMatrixTranslation(pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2]);
	}

	inline XMMATRIX CreateViewMatrix(XMVECTOR pos, float yaw, float pitch)
	{
		return XMMatrixTranslation(-pos.m128_f32[0], -pos.m128_f32[1], -pos.m128_f32[2])
			* XMMatrixRotationY(-yaw)
			* XMMatrixRotationX(-pitch);
	}
} // namespace math