#pragma once

#include "Math.hpp"

namespace h2r
{
	struct HostConstantBuffer
	{
		XMMATRIX world;
		XMMATRIX worldViewProj;
		XMVECTOR cameraWorldPos;
	};

	struct MaterialConstants
	{
		XMFLOAT3 ambient;
		float padd0;
		XMFLOAT3 diffuse;
		float padd1;
		XMFLOAT3 specular;
		float shininess;
	};
}
