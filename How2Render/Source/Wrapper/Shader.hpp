#pragma once

#include "Helpers/ShaderLoader.hpp"
#include "Wrapper/BlendState.hpp"
#include "Wrapper/ConstantBuffer.hpp"
#include "Wrapper/Sampler.hpp"
#include <d3d11.h>
#include <filesystem>

namespace h2r
{

	struct ShadersDescriptor
	{
		std::filesystem::path vertexShaderPath;
		std::filesystem::path pixelShaderPath;
	};

	struct ShaderProgram
	{
		ID3D11VertexShader *pVertexShader = nullptr;
		ID3D11PixelShader *pPixelShader = nullptr;
		ID3D11InputLayout *pVertexLayout = nullptr;
	};

	struct ForwardShaders
	{
		ShaderProgram shadingPass;
		ShaderProgram translucentPass;
		DeviceConstBuffers cbuffers;
		TextureSamplers samplers;
		BlendStates blendStates;
	};

	inline void BindShaders(
		Context const &context,
		ShaderProgram const &shaders)
	{
		context.pImmediateContext->VSSetShader(shaders.pVertexShader, nullptr, 0);
		context.pImmediateContext->PSSetShader(shaders.pPixelShader, nullptr, 0);
	}

	inline ShaderProgram CreateShaderProgram(Context const &context, ShadersDescriptor &desc)
	{
		ShaderProgram shaders = {};

		// Create vertex shader and set up vertex layout
		{
			ID3DBlob *pVSBlob = nullptr;
			auto hr = CompileShaderFromFile(desc.pixelShaderPath.c_str(), "VS", "vs_4_0", &pVSBlob);
			if (FAILED(hr))
			{
				printf("Failed to compile vertex shader from file");
				assert(SUCCEEDED(hr));
			}

			hr = context.pd3dDevice->CreateVertexShader(
				pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &shaders.pVertexShader);
			if (FAILED(hr))
			{
				printf("Failed to create vertex shader");
				assert(SUCCEEDED(hr));
			}

			D3D11_INPUT_ELEMENT_DESC const layout[] =
				{
					{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
					{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
					{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
				};
			hr = context.pd3dDevice->CreateInputLayout(
				layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &shaders.pVertexLayout);
			if (FAILED(hr))
			{
				printf("Failed to create input layout");
				assert(SUCCEEDED(hr));
			}
			context.pImmediateContext->IASetInputLayout(shaders.pVertexLayout);

			pVSBlob->Release();
		}

		// Create pixel shader
		{
			ID3DBlob *pPSBlob = nullptr;
			auto hr = CompileShaderFromFile(desc.pixelShaderPath.c_str(), "PS", "ps_4_0", &pPSBlob);
			if (FAILED(hr))
			{
				wprintf(L"Failed to compile pixel shader from file: '%s'", desc.pixelShaderPath.c_str());
				assert(SUCCEEDED(hr));
			}

			hr = context.pd3dDevice->CreatePixelShader(
				pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &shaders.pPixelShader);
			if (FAILED(hr))
			{
				printf("Failed to create pixel shader");
				assert(SUCCEEDED(hr));
			}
			pPSBlob->Release();
		}

		return shaders;
	}

	inline void CleanupShaders(ShaderProgram &shaders)
	{
		if (shaders.pVertexShader != nullptr)
		{
			shaders.pVertexShader->Release();
			shaders.pVertexShader = nullptr;
		}
		if (shaders.pPixelShader != nullptr)
		{
			shaders.pPixelShader->Release();
			shaders.pPixelShader = nullptr;
		}
		if (shaders.pVertexLayout != nullptr)
		{
			shaders.pVertexLayout->Release();
			shaders.pVertexLayout = nullptr;
		}
	};

	inline ForwardShaders CreateForwardShaders(Context const &context)
	{
		ShadersDescriptor shadingPassDesc;
		shadingPassDesc.vertexShaderPath = L"Shaders/ForwardShading.fx";
		shadingPassDesc.pixelShaderPath = L"Shaders/ForwardShading.fx";

		ShadersDescriptor translucencyPassDesc;
		translucencyPassDesc.vertexShaderPath = L"Shaders/Translucent.fx";
		translucencyPassDesc.pixelShaderPath = L"Shaders/Translucent.fx";

		ForwardShaders forwardShaders;
		forwardShaders.shadingPass = CreateShaderProgram(context, shadingPassDesc);
		forwardShaders.translucentPass = CreateShaderProgram(context, translucencyPassDesc);

		forwardShaders.cbuffers = CreateDeviceConstantBuffers(context);
		forwardShaders.samplers = CreateTextureSamplers(context);
		forwardShaders.blendStates = CreateBlendStates(context);

		return forwardShaders;
	}

	inline void CleanupForwardShaders(ForwardShaders &forwardShaders)
	{
		CleanupBlendStates(forwardShaders.blendStates);
		CleanupTextureSamplers(forwardShaders.samplers);

		CleanupShaders(forwardShaders.shadingPass);
		CleanupShaders(forwardShaders.translucentPass);
	}

} // namespace h2r
