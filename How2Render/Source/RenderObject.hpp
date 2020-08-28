#pragma once

#include "Mesh.hpp"
#include "Material.hpp"

namespace h2r
{

	struct RenderObject
	{
		DeviceMesh mesh;
		DeviceMaterial material;
		XMMATRIX world;
	};

	void CleanupRenderObject(RenderObject& renderObject)
	{
		CleanupDeviceMesh(renderObject.mesh);
		ClenupMaterial(renderObject.material);
	}

}
