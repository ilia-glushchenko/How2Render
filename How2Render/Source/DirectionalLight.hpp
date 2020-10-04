#pragma once

#include "Math.hpp"
#include "Wrapper/Context.hpp"

namespace h2r
{

	struct DirectionalLight
	{
		XMVECTOR direction = {};
		float viewLeft = -1.f;
		float viewRight = 1.f;
		float viewBottom = -1.f;
		float viewTop = 1.f;
		float zNear = 1.f;
		float zFar = 50.f;
		XMMATRIX view = {};
		XMMATRIX proj = {};
		XMMATRIX viewProj = {};
	};

	inline DirectionalLight CreateDirectionalLight(Context const &context);

} // namespace h2r

namespace h2r
{

	inline void CalculateViewProjection(DirectionalLight &lightSource)
	{
		XMVECTOR const upDirection = XMVectorSet(0.f, 0.f, 1.f, 1.f);
		XMVECTOR const centroid = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		float const length = lightSource.zFar / 2.f - lightSource.zNear;
		XMVECTOR scaledDir = XMVectorScale(lightSource.direction, length);
		XMVECTOR position = XMVectorAdd(centroid, scaledDir);

		lightSource.view = XMMatrixLookAtLH(position, centroid, upDirection);

		lightSource.proj = XMMatrixOrthographicOffCenterLH(
			lightSource.viewLeft,
			lightSource.viewRight,
			lightSource.viewBottom,
			lightSource.viewTop,
			lightSource.zNear,
			lightSource.zFar);
		lightSource.viewProj = XMMatrixMultiply(lightSource.view, lightSource.proj);
	}

	inline DirectionalLight CreateDirectionalLight(Context const &context)
	{
		DirectionalLight lightSource;

		lightSource.direction = XMVector3Normalize({0.f, 1.f, 0.f, 0.f});
		lightSource.viewLeft = -25.f;
		lightSource.viewRight = -lightSource.viewLeft;
		lightSource.viewBottom = -25.f;
		lightSource.viewTop = -lightSource.viewBottom;
		lightSource.zNear = 1.f;
		lightSource.zFar = 50.f;

		CalculateViewProjection(lightSource);

		return lightSource;
	}

} // namespace h2r