#pragma once

#include "Shader.hpp"
#include "Window.hpp"

#include <SDL.h>
#include <SDL_syswm.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include <cstdio>
#include <glm/glm.hpp>
#include <tuple>

struct ConstantBuffer
{
	DirectX::XMMATRIX view;
	uint32_t screenWidth;
	uint32_t screenHeight;
	uint32_t frameCount;
};

struct Application
{
	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	ID3D11Device *pd3dDevice = nullptr;
	ID3D11DeviceContext *pImmediateContext = nullptr;
	IDXGISwapChain *pSwapChain = nullptr;
	ID3D11Texture2D* pBackBuffer = nullptr;
	ID3D11RenderTargetView *pRenderTargetView = nullptr;

	ID3D11VertexShader *pVertexShader = nullptr;
	ID3D11PixelShader *pPixelShader = nullptr;

	ID3D11InputLayout *pVertexLayout = nullptr;
	ID3D11Buffer *pVertexBuffer = nullptr;

	ID3D11Buffer* pConstantBuffer = nullptr;
};

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 ui;
};

inline std::tuple<bool, Application> CreateApplication(Window &window)
{
	Application app;
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes;
		 driverTypeIndex++)
	{
		app.driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, app.driverType, nullptr, createDeviceFlags,
							   featureLevels, numFeatureLevels, D3D11_SDK_VERSION,
							   &app.pd3dDevice, &app.featureLevel,
							   &app.pImmediateContext);

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
	{
		printf("Failed to create device.\n");
		return {false, app};
	}

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory *dxgiFactory = nullptr;
	{
		IDXGIDevice *dxgiDevice = nullptr;
		hr = app.pd3dDevice->QueryInterface(__uuidof(IDXGIDevice),
											reinterpret_cast<void **>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter *adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory),
										reinterpret_cast<void **>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
	{
		printf("Failed to create factory.\n");
		return {false, app};
	}

	glm::vec2 const win_size = GetWindowSize(window);
	SDL_SysWMinfo sysWMInfo;
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(window.window, &sysWMInfo);

	// Create swap chain
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = static_cast<uint32_t>(win_size.x);
	sd.BufferDesc.Height = static_cast<uint32_t>(win_size.y);
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = sysWMInfo.info.win.window;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	hr = dxgiFactory->CreateSwapChain(app.pd3dDevice, &sd, &app.pSwapChain);
	dxgiFactory->MakeWindowAssociation(sysWMInfo.info.win.window,
									   DXGI_MWA_NO_ALT_ENTER);
	dxgiFactory->Release();
	if (FAILED(hr))
	{
		printf("Failed to create swap chain.");
		return {false, app};
	}

	// Create a render target view
	hr = app.pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
								   reinterpret_cast<void **>(&app.pBackBuffer));
	if (FAILED(hr))
	{
		printf("Failed to create render target buffer.");
		return {false, app};
	}

	hr = app.pd3dDevice->CreateRenderTargetView(app.pBackBuffer, nullptr,
												&app.pRenderTargetView);

	if (FAILED(hr))
	{
		printf("Failed to create render target view.");
		return {false, app};
	}

	app.pImmediateContext->OMSetRenderTargets(1, &app.pRenderTargetView, nullptr);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)win_size.x;
	vp.Height = (FLOAT)win_size.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	app.pImmediateContext->RSSetViewports(1, &vp);

	// Compile the vertex shader
	ID3DBlob *pVSBlob = nullptr;
	hr = CompileShaderFromFile(L"Shaders/Lecture3/Lecture03.fx", "VS", "vs_4_0",
							   &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
				   L"The FX file cannot be compiled. Please run this executable "
				   L"from the directory that contains the FX file.",
				   L"Error", MB_OK);
		return {false, app};
	}

	// Create the vertex shader
	hr = app.pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),
											pVSBlob->GetBufferSize(), nullptr,
											&app.pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		printf("Failed to create vertex shader.");
		return {false, app};
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	
	// Create the input layout
	hr = app.pd3dDevice->CreateInputLayout(
		layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &app.pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
	{
		printf("Failed to create vertex shader input layout.");
		return {false, app};
	}

	// Set the input layout
	app.pImmediateContext->IASetInputLayout(app.pVertexLayout);

	// Compile the pixel shader
	ID3DBlob *pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Shaders/Lecture3/Lecture03.fx", "PS", "ps_4_0",
							   &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
				   L"The FX file cannot be compiled. Please run this executable "
				   L"from the directory that contains the FX file.",
				   L"Error", MB_OK);
		return {false, app};
	}

	// Create the pixel shader
	hr = app.pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
										   pPSBlob->GetBufferSize(), nullptr,
										   &app.pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
	{
		printf("Failed to create pixel shader.");
		return {false, app};
	}

	// Create vertex buffer
	Vertex vertices[3] = {
		Vertex{
			DirectX::XMFLOAT3{-1.0, 3.0, 0.5},
			DirectX::XMFLOAT2{0.0, -1.0},
		},
		Vertex{
			DirectX::XMFLOAT3{3.0, -1.0, 0.5},
			DirectX::XMFLOAT2{2.0, 1.0},
		},
		Vertex{
			DirectX::XMFLOAT3{-1.0, -1.0, 0.5},
			DirectX::XMFLOAT2{0.0, 1.0},
		},
	};
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * 3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = app.pd3dDevice->CreateBuffer(&bd, &InitData, &app.pVertexBuffer);
	if (FAILED(hr))
	{
		printf("Failed to create vertex buffer.");
		return {false, app};
	}

	// Set vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	app.pImmediateContext->IASetVertexBuffers(0, 1, &app.pVertexBuffer, &stride,
											  &offset);

	// Set primitive topology
	app.pImmediateContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = app.pd3dDevice->CreateBuffer(&bd, nullptr, &app.pConstantBuffer);
	if (FAILED(hr)) {
		printf("Failed to create constant buffer.");
		return { false, app };
	}

	return {true, app};
}

inline void CleanupDevice(Application &app)
{
	if (app.pImmediateContext)
	{
		app.pImmediateContext->ClearState();
	}

	if (app.pVertexBuffer)
	{
		app.pVertexBuffer->Release();
	}
	if (app.pVertexLayout)
	{
		app.pVertexLayout->Release();
	}
	if (app.pVertexShader)
	{
		app.pVertexShader->Release();
	}
	if (app.pPixelShader)
	{
		app.pPixelShader->Release();
	}
	if (app.pRenderTargetView)
	{
		app.pRenderTargetView->Release();
	}
	if (app.pSwapChain)
	{
		app.pSwapChain->Release();
	}
	if (app.pImmediateContext)
	{
		app.pImmediateContext->Release();
	}
	if (app.pd3dDevice)
	{
		app.pd3dDevice->Release();
	}
}