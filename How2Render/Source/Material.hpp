#pragma once

#include "Wrapper/Shader.hpp"
#include "Wrapper/Texture.hpp"
#include "Wrapper/Sampler.hpp"
#include "Wrapper/BlendState.hpp"

namespace h2r
{
	constexpr int InvalidMaterialId = -1;
	constexpr uint32_t MaxMaterialTextures = 4;

	struct DeviceMaterial
	{
		DeviceTexture ambientTexture;
		DeviceTexture albedoTexture;
		DeviceTexture specularTexture;
		XMFLOAT3 ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);
		XMFLOAT3 diffuse = XMFLOAT3(1.f, 1.f, 1.f);
		XMFLOAT3 specular = XMFLOAT3(0.f, 0.f, 0.f);
		float shininess = 1.f;
		bool enableBlend = false;
	};

	void SetMaterial(DeviceMaterial const& material, Context const& context, Shaders const& shaders, BlendStates const& blendStates)
	{
		MaterialConstantBuffer materialConstants;
		ID3D11ShaderResourceView *shaderResourceViews[MaxMaterialTextures];

		shaderResourceViews[0] = material.ambientTexture.pShaderResourceView;
		shaderResourceViews[1] = material.albedoTexture.pShaderResourceView;
		shaderResourceViews[2] = material.specularTexture.pShaderResourceView;
		shaderResourceViews[3] = nullptr;

		materialConstants.ambient = material.ambient;
		materialConstants.diffuse = material.diffuse;
		materialConstants.specular = material.specular;
		materialConstants.shininess = material.shininess;

		// Update material properties
		context.pImmediateContext->UpdateSubresource(shaders.pMaterialConstants, 0, nullptr, &materialConstants, 0, 0);

		// Set material textures
		context.pImmediateContext->PSSetShaderResources(0, MaxMaterialTextures, shaderResourceViews);

		if (material.enableBlend)
		{
			// Enable alpha-to-coverage blending
			constexpr float blendFactor[4] = {1.f, 1.f, 1.f, 1.f};
			context.pImmediateContext->OMSetBlendState(blendStates.pNormalBlendState, blendFactor, 0xFFFFFFFF);
		}
		else
		{
			// Disable blending
			context.pImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
		}
	}

	void SetNullMaterial(Context const& context, Shaders const& shaders)
	{
		MaterialConstantBuffer materialConstants;
		ID3D11ShaderResourceView *shaderResourceViews[MaxMaterialTextures];

		for (int i = 0; i < MaxMaterialTextures; ++i)
			shaderResourceViews[i] = nullptr;

		materialConstants.ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);
		materialConstants.diffuse = XMFLOAT3(1.f, 1.f, 1.f);
		materialConstants.specular = XMFLOAT3(1.f, 1.f, 1.f);
		materialConstants.shininess = 0.f;

		// Update material properties
		context.pImmediateContext->UpdateSubresource(shaders.pMaterialConstants, 0, nullptr, &materialConstants, 0, 0);

		// Set material textures
		context.pImmediateContext->PSSetShaderResources(0, MaxMaterialTextures, shaderResourceViews);

		// Disable blending
		context.pImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	}
} // namespace h2r
