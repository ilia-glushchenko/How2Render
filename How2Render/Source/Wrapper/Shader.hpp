#pragma once

#include "Helpers/ShaderLoader.hpp"
#include "Input.hpp"
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

	struct Shaders
	{
		ShaderProgram forwardShading;

		ShaderProgram gBufferPass;
		ShaderProgram deferredShading;

		ShaderProgram translucentPass;
		ShaderProgram gammaCorrection;

		DeviceConstBuffers cbuffers;
		TextureSamplers samplers;
		BlendStates blendStates;
	};

	inline std::optional<Shaders> CreateShaders(Context const &context);

	inline void CleanupShaders(Shaders &forwardShaders);

	inline void BindShaders(Context const &context, ShaderProgram const &shaders);

	inline std::optional<ShaderProgram> CreateShaderProgram(Context const &context, ShadersDescriptor &desc);

	inline void CleanupShaderProgram(ShaderProgram &shaders);

	inline void HotReloadeShaders(Context const& context, InputEvents const& inputs, Shaders& shaders);

} // namespace h2r

namespace h2r
{
	inline void BindShaders(Context const &context, ShaderProgram const &shaders)
	{
		context.pImmediateContext->VSSetShader(shaders.pVertexShader, nullptr, 0);
		context.pImmediateContext->PSSetShader(shaders.pPixelShader, nullptr, 0);
	}

	inline std::optional<ShaderProgram> CreateShaderProgram(Context const &context, ShadersDescriptor &desc)
	{
		ShaderProgram shaders = {};

		// Create vertex shader and set up vertex layout
		{
			ID3DBlob *pVSBlob = nullptr;
			auto hr = CompileShaderFromFile(desc.pixelShaderPath.c_str(), "VS", "vs_4_0", &pVSBlob);
			if (FAILED(hr))
			{
				printf("Failed to compile vertex shader from file");
				return std::nullopt;
			}

			hr = context.pd3dDevice->CreateVertexShader(
				pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &shaders.pVertexShader);
			if (FAILED(hr))
			{
				printf("Failed to create vertex shader");
				pVSBlob->Release();
				return std::nullopt;
			}

			D3D11_INPUT_ELEMENT_DESC const layout[] =
				{
					{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
					{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
					{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
				};
			hr = context.pd3dDevice->CreateInputLayout(
				layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &shaders.pVertexLayout);
			pVSBlob->Release();

			if (FAILED(hr))
			{
				printf("Failed to create input layout");
				CleanupShaderProgram(shaders);
				return std::nullopt;
			}
			context.pImmediateContext->IASetInputLayout(shaders.pVertexLayout);
		}

		// Create pixel shader
		{
			ID3DBlob *pPSBlob = nullptr;
			auto hr = CompileShaderFromFile(desc.pixelShaderPath.c_str(), "PS", "ps_4_0", &pPSBlob);
			if (FAILED(hr))
			{
				wprintf(L"Failed to compile pixel shader from file: '%s'", desc.pixelShaderPath.c_str());
				pPSBlob->Release();
				CleanupShaderProgram(shaders);
				return std::nullopt;
			}

			hr = context.pd3dDevice->CreatePixelShader(
				pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &shaders.pPixelShader);
			pPSBlob->Release();

			if (FAILED(hr))
			{
				printf("Failed to create pixel shader");
				CleanupShaderProgram(shaders);
				return std::nullopt;
			}
		}

		return shaders;
	}

	inline void CleanupShaderProgram(ShaderProgram &shaders)
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

	inline std::optional<Shaders> CreateShaders(Context const &context)
	{
		ShadersDescriptor forwardShadingPassDesc;
		forwardShadingPassDesc.vertexShaderPath = "Shaders/ForwardShading.fx";
		forwardShadingPassDesc.pixelShaderPath = "Shaders/ForwardShading.fx";

		ShadersDescriptor gBufferPassDesc;
		gBufferPassDesc.vertexShaderPath = "Shaders/DeferredGBufferPass.fx";
		gBufferPassDesc.pixelShaderPath = "Shaders/DeferredGBufferPass.fx";

		ShadersDescriptor deferredShadingPassDesc;
		deferredShadingPassDesc.vertexShaderPath = "Shaders/DeferredShading.fx";
		deferredShadingPassDesc.pixelShaderPath = "Shaders/DeferredShading.fx";

		ShadersDescriptor translucencyPassDesc;
		translucencyPassDesc.vertexShaderPath = "Shaders/Translucent.fx";
		translucencyPassDesc.pixelShaderPath = "Shaders/Translucent.fx";

		ShadersDescriptor gammaCorrectionDesc;
		gammaCorrectionDesc.vertexShaderPath = "Shaders/GammaCorrection.fx";
		gammaCorrectionDesc.pixelShaderPath = "Shaders/GammaCorrection.fx";

		Shaders shaders;
		if (auto forwardShading = CreateShaderProgram(context, forwardShadingPassDesc); forwardShading)
		{
			shaders.forwardShading = forwardShading.value();
		}
		else
		{
			CleanupShaders(shaders);
			return std::nullopt;
		}
		if (auto gBufferPass = CreateShaderProgram(context, gBufferPassDesc); gBufferPass)
		{
			shaders.gBufferPass = gBufferPass.value();
		}
		else
		{
			CleanupShaders(shaders);
			return std::nullopt;
		}
		if (auto deferredShading = CreateShaderProgram(context, deferredShadingPassDesc); deferredShading)
		{
			shaders.deferredShading = deferredShading.value();
		}
		else
		{
			CleanupShaders(shaders);
			return std::nullopt;
		}
		if (auto translucentPass = CreateShaderProgram(context, translucencyPassDesc); translucentPass)
		{
			shaders.translucentPass = translucentPass.value();
		}
		else
		{
			CleanupShaders(shaders);
			return std::nullopt;
		}
		if (auto gammaCorrection = CreateShaderProgram(context, gammaCorrectionDesc); gammaCorrection)
		{
			shaders.gammaCorrection = gammaCorrection.value();
		}
		else
		{
			CleanupShaders(shaders);
			return std::nullopt;
		}

		shaders.cbuffers = CreateDeviceConstantBuffers(context);
		shaders.samplers = CreateTextureSamplers(context);
		shaders.blendStates = CreateBlendStates(context);

		return shaders;
	}

	inline void CleanupShaders(Shaders &shaders)
	{
		CleanupBlendStates(shaders.blendStates);
		CleanupTextureSamplers(shaders.samplers);

		CleanupShaderProgram(shaders.forwardShading);
		CleanupShaderProgram(shaders.gBufferPass);
		CleanupShaderProgram(shaders.deferredShading);
		CleanupShaderProgram(shaders.translucentPass);
		CleanupShaderProgram(shaders.gammaCorrection);
	}

	inline void HotReloadeShaders(Context const& context, InputEvents const& inputs, Shaders& shaders)
	{
		if (IsKeyDown(inputs, SDL_SCANCODE_F5))
		{
			auto newShaders = CreateShaders(context);
			if (newShaders)
			{
				CleanupShaders(shaders);
				shaders = newShaders.value();
			}
			else
			{
				printf("Shader hot reload failed\n");
			}
		}
	}

} // namespace h2r
