#pragma once

#include <d3d11.h>
#include <cstdint>
#include <cassert>

struct Context
{
	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* pd3dDevice = nullptr;
	ID3D11DeviceContext* pImmediateContext = nullptr;
};

inline Context CreateContext()
{
	Context context;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE const driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	uint32_t const numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL const featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
	};
	uint32_t const numFeatureLevels = ARRAYSIZE(featureLevels);

	HRESULT hr = S_OK;
	for (uint32_t driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		context.driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(
			nullptr,
			context.driverType,
			nullptr,
			createDeviceFlags,
			featureLevels,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&context.pd3dDevice,
			&context.featureLevel,
			&context.pImmediateContext);

		if (SUCCEEDED(hr)) {
			break;
		}
	}
	assert(SUCCEEDED(hr));

	return context;
}

void CleanupContext(Context const& context)
{
	if (context.pd3dDevice != nullptr)
	{
		context.pd3dDevice->Release();
	}
	if (context.pImmediateContext != nullptr)
	{
		context.pImmediateContext->Release();
	}
};