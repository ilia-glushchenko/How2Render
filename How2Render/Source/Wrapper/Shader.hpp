#pragma once

#include "Wrapper/Context.hpp"
#include "Wrapper/ConstantBuffer.hpp"

#include <d3d11.h>
#include <d3dcompiler.h>

struct Shaders
{
	ID3D11VertexShader *pVertexShader = nullptr;
	ID3D11PixelShader *pPixelShader = nullptr;
	ID3D11InputLayout *pVertexLayout = nullptr;
	ID3D11Buffer *pConstantBuffer = nullptr;
};

inline HRESULT CompileShaderFromFile(const WCHAR *szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob **ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows
	// the shaders to be optimized and to run exactly the way they will run in
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob *pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
							dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char *>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob)
		pErrorBlob->Release();

	return S_OK;
}

inline Shaders CreateShaders(Context &context)
{
	Shaders shaders;

	// Compile the vertex shader
	ID3DBlob *pVSBlob = nullptr;
	auto hr = CompileShaderFromFile(L"Shaders/Lecture3/Lecture03.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
				   L"The FX file cannot be compiled. Please run this executable "
				   L"from the directory that contains the FX file.",
				   L"Error", MB_OK);
	}

	// Create the vertex shader
	hr = context.pd3dDevice->CreateVertexShader(
		pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &shaders.pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		printf("Failed to create vertex shader.");
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = context.pd3dDevice->CreateInputLayout(
		layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &shaders.pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
	{
		printf("Failed to create vertex shader input layout.");
	}

	// Set the input layout
	context.pImmediateContext->IASetInputLayout(shaders.pVertexLayout);

	// Compile the pixel shader
	ID3DBlob *pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Shaders/Lecture3/Lecture03.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
				   L"The FX file cannot be compiled. Please run this executable "
				   L"from the directory that contains the FX file.",
				   L"Error", MB_OK);
	}

	// Create the pixel shader
	hr = context.pd3dDevice->CreatePixelShader(
		pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		nullptr,
		&shaders.pPixelShader);
	pPSBlob->Release();
	assert(SUCCEEDED(hr));

	// Create the constant buffer
	D3D11_BUFFER_DESC bufferDescriptor = {};
	bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
	bufferDescriptor.ByteWidth = sizeof(HostConstantBuffer);
	bufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescriptor.CPUAccessFlags = 0;
	hr = context.pd3dDevice->CreateBuffer(&bufferDescriptor, nullptr, &shaders.pConstantBuffer);
	assert(SUCCEEDED(hr));

	return shaders;
}

void CleanupShaders(Shaders const &shaders)
{
	if (shaders.pVertexShader != nullptr)
	{
		shaders.pVertexShader->Release();
	}
	if (shaders.pPixelShader != nullptr)
	{
		shaders.pPixelShader->Release();
	}
	if (shaders.pConstantBuffer != nullptr)
	{
		shaders.pConstantBuffer->Release();
	}
	if (shaders.pVertexLayout != nullptr)
	{
		shaders.pVertexLayout->Release();
	}
};
