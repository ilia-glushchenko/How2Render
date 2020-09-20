#pragma once

#include "Application.hpp"
#include "RenderCommon.hpp"
#include "RenderObject.hpp"
#include "Wrapper/Context.hpp"
#include "Wrapper/RenderTarget.hpp"
#include "Wrapper/Shader.hpp"
#include "Wrapper/Texture.hpp"

namespace h2r
{

	void GBufferPass(
		Context const &context,
		Shaders const &shaders,
		Application::States const &states,
		std::vector<RenderObject> &objects)
	{
		// Draw opaque
		if (states.drawOpaque)
		{
			BindBlendState(context, shaders.blendStates.none);
			BindShaders(context, shaders.gBufferPass);
			BindConstantBuffers(context, shaders.cbuffers);
			BindSamplers(context, shaders.samplers);

			for (auto const &object : objects)
			{
				for (auto const &mesh : object.model.opaqueMeshes)
				{
					UpdatePerMeshConstantBuffer(
						context,
						shaders.cbuffers,
						object.model.materials,
						mesh,
						object.transform);

					Draw(context, mesh);
				}
			}
		}

		// Draw transparent
		if (states.drawTransparent)
		{
			BindBlendState(context, shaders.blendStates.alphaToCoverage);
			BindShaders(context, shaders.gBufferPass);
			BindConstantBuffers(context, shaders.cbuffers);
			BindSamplers(context, shaders.samplers);

			for (auto const &object : objects)
			{
				for (auto const &mesh : object.model.transparentMeshes)
				{
					UpdatePerMeshConstantBuffer(
						context,
						shaders.cbuffers,
						object.model.materials,
						mesh,
						object.transform);

					Draw(context, mesh);
				}
			}
		}
	}

	void ShadeDeferred(
		Context const &context,
		Shaders const &shaders,
		Application::States const &states,
		RenderTargets::GBuffers const &gbuffers)
	{
		constexpr uint32_t gbufferCount = sizeof(RenderTargets::GBuffers) / sizeof(DeviceTexture);
		ID3D11ShaderResourceView *views[gbufferCount] = {
			gbuffers.positionTexture.shaderResourceView,
			gbuffers.normalTexture.shaderResourceView,
			gbuffers.ambientTexture.shaderResourceView,
			gbuffers.diffuseTexture.shaderResourceView,
			gbuffers.specularTexture.shaderResourceView,
			gbuffers.shininessTexture.shaderResourceView,
		};

		DrawFullScreen(context, shaders.deferredShading, shaders.blendStates.none, shaders.samplers, shaders.cbuffers, gbufferCount, views);
	}

} // namespace h2r
