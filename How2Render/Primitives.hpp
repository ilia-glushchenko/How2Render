#pragma once

#include <DirectXMath.h>

struct Sphere
{
	DirectX::XMFLOAT3 albedo;
	DirectX::XMVECTOR position;
	float radius;
};

struct Ray
{
	DirectX::XMVECTOR origin;
	DirectX::XMVECTOR direction;
};