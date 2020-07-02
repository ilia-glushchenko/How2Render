#pragma once

#include "Math.hpp"

struct HostConstantBuffer
{
	XMMATRIX view;
	uint32_t screenWidth;
	uint32_t screenHeight;
	uint32_t frameCount;
};
