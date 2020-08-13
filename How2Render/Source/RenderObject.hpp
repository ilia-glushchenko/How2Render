#pragma once

#include "Math.hpp"
#include "ObjModelLoader.hpp"

namespace h2r
{

	struct RenderObject
	{
		DeviceModel model;
		XMMATRIX world;
	};

	std::tuple<bool, RenderObject> CreateRenderObject(std::string const &fileName, TextureLoader &loader, float scale)
	{
		auto [result, objModel] = LoadObjModel(fileName, loader);
		if (!result)
		{
			return {false, RenderObject{}};
		}

		RenderObject renderObject;
		renderObject.model = objModel;
		renderObject.world = XMMatrixScaling(scale, scale, scale);

		return {true, renderObject};
	}

	void FreeRenderObject(RenderObject &renderObject)
	{
		FreeModel(renderObject.model);
	}

} // namespace h2r
