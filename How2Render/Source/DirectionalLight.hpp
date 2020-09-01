#pragma once

#include "Wrapper/DepthRenderTarget.hpp"
#include "Math.hpp"

namespace h2r
{

	struct DirectionalLight
	{
		XMVECTOR direction;
		float viewLeft = -1.f;
		float viewRight = 1.f;
		float viewBottom = -1.f;
		float viewTop = 1.f;
		float zNear = 1.f;
		float zFar = 1000.f;
		XMMATRIX view;
		XMMATRIX proj;
		XMMATRIX viewProj;
		DepthRenderTarget shadowMap;
	};

	void CalculateViewProjection(DirectionalLight& lightSource)
	{
		XMVECTOR const upDirection = XMVectorSet(0.f, 1.f, 0.f, 1.f);
		XMVECTOR const centroid = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		float const length = lightSource.zFar/2.f - lightSource.zNear;
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

	XMMATRIX CalculateShadowProjection(DirectionalLight const& lightSource)
	{
		// [-1,1] -> [0,1], flip V coord
		static XMMATRIX const bias = XMMatrixSet(
			.5f, .0f, 0.f, 0.f,
			.0f,-.5f, 0.f, 0.f,
			.0f, .0f, 1.f, 0.f,
			.5f, .5f, 0.f, 1.f);

		XMMATRIX shadowProj = XMMatrixMultiply(lightSource.viewProj, bias);
		return shadowProj;
	}

	DirectionalLight CreateDirectionalLight(Context const& context)
	{
		DirectionalLight lightSource;

		lightSource.direction = XMVector3Normalize(XMVectorSet(-2.f, 3.f, 2.f, 0.f));
		lightSource.viewLeft = -100.f;
		lightSource.viewRight = -lightSource.viewLeft;
		lightSource.viewBottom = -100.f;
		lightSource.viewTop = -lightSource.viewBottom;
		lightSource.zNear = 10.f;
		lightSource.zFar = 500.f;

		CalculateViewProjection(lightSource);

		constexpr uint32_t shadowMapSize = 2048;
		constexpr eDepthPrecision lowPrecision = eDepthPrecision::Unorm16; // Low precision is enough

		auto [result, shadowMap] = CreateDepthRenderTarget(context, shadowMapSize, shadowMapSize, lowPrecision);
		assert(result);
		lightSource.shadowMap = shadowMap;

		return lightSource;
	}

	void FreeDirectionalLight(DirectionalLight& lightSource)
	{
		ReleaseDepthRenderTarget(lightSource.shadowMap);
	}

} // namespace h2r
