#pragma once

#include "Wrapper/DepthRenderTarget.hpp"
#include "Math.hpp"

namespace h2r
{

	struct SpotLight
	{
		XMVECTOR position;
		XMVECTOR lookAtPosition;
		float fov = 45.f;
		float zNear = 1.f;
		float zFar = 1000.f;
		XMMATRIX view;
		XMMATRIX proj;
		XMMATRIX viewProj;
		DepthRenderTarget shadowMap;
	};

	void CalculateViewProjection(SpotLight& spotLight)
	{
		XMVECTOR const upDirection = XMVectorSet(0.f, 1.f, 0.f, 1.f);
		float const fovRadians = XMConvertToRadians(spotLight.fov);

		spotLight.view = XMMatrixLookAtLH(spotLight.position, spotLight.lookAtPosition, upDirection);
		spotLight.proj = XMMatrixPerspectiveFovLH(fovRadians, 1.f, spotLight.zNear, spotLight.zFar);

		spotLight.viewProj = XMMatrixMultiply(spotLight.view, spotLight.proj);
	}

	XMMATRIX CalculateShadowProjection(SpotLight const& spotLight)
	{
		// [-1,1] -> [0,1], flip V coord
		static XMMATRIX const bias = XMMatrixSet(
			.5f, .0f, 0.f, 0.f,
			.0f,-.5f, 0.f, 0.f,
			.0f, .0f, 1.f, 0.f,
			.5f, .5f, 0.f, 1.f);

		XMMATRIX shadowProj = XMMatrixMultiply(spotLight.viewProj, bias);
		return shadowProj;
	}

	SpotLight CreateDefaultSpotLight(Context const& context)
	{
		SpotLight spotLight;

		spotLight.position = XMVectorSet(-200.0f, 300.f, 200.f, 1.f);
		spotLight.lookAtPosition = XMVectorSet(0.f, 25.f, 0.f, 1.f);
		spotLight.fov = 13.f;
		spotLight.zNear = 100.f;
		spotLight.zFar = 1000.f;

		CalculateViewProjection(spotLight);

		constexpr uint32_t shadowMapSize = 2048;
		constexpr eDepthPrecision lowPrecision = eDepthPrecision::Unorm16; // Low precision is enough

		auto [result, shadowMap] = CreateDepthRenderTarget(context, shadowMapSize, shadowMapSize, lowPrecision);
		assert(result);
		spotLight.shadowMap = shadowMap;

		return spotLight;
	}

	void FreeSpotLight(SpotLight& spotLight)
	{
		ReleaseDepthRenderTarget(spotLight.shadowMap);
	}

} // namespace h2r
