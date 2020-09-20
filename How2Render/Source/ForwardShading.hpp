#pragma once

#include "Application.hpp"
#include "Camera.hpp"
#include "Helpers/MeshGenerator.hpp"
#include "RenderCommon.hpp"
#include "Window.hpp"
#include "Wrapper/Shader.hpp"

namespace h2r
{

	inline void ShadeForward(
		Context const &context,
		Shaders const &shaders,
		Application::States const &states,
		std::vector<RenderObject> const &objects)
	{
		// Draw opaque
		if (states.drawOpaque)
		{
			BindBlendState(context, shaders.blendStates.none);
			BindShaders(context, shaders.forwardShading);
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
			BindShaders(context, shaders.forwardShading);
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

} // namespace h2r