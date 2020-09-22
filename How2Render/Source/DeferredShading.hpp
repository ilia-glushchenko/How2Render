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
			BindSampler(context, shaders.samplers.pTrilinearSampler);

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
	}

	void ShadeDeferred(
		Context const &context,
		Shaders const &shaders,
		Application::States const &states,
		RenderTargets::GBuffers const &gbuffers,
		Swapchain const &swapchain)
	{
		constexpr uint32_t gbufferCount = sizeof(RenderTargets::GBuffers) / sizeof(DeviceTexture);
		ID3D11ShaderResourceView *views[gbufferCount + 1] = {
			swapchain.depthStencilShaderResourceView,
			gbuffers.ambientTexture.shaderResourceView,
			gbuffers.diffuseTexture.shaderResourceView,
			gbuffers.specularTexture.shaderResourceView,
		};

		DrawFullScreen(
			context,
			shaders.deferredShading,
			shaders.blendStates.none,
			*shaders.samplers.pPointSampler,
			shaders.cbuffers,
			gbufferCount + 1,
			views);
	}

} // namespace h2r
