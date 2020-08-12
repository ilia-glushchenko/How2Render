#pragma once
#include "ThirdParty/tinyobjloader/tiny_obj_loader.h"
#include "TextureLoader.hpp"

namespace h2r
{
	struct ObjMaterial
	{
		DeviceTexture ambientTexture;
		DeviceTexture albedoTexture;
		DeviceTexture specularTexture;
		XMFLOAT3 ambient;
		XMFLOAT3 diffuse;
		XMFLOAT3 specular;
		float shininess;
	};

	ObjMaterial LoadObjMaterial(tinyobj::material_t const& mat, TextureLoader& loader)
	{
		ObjMaterial material;

		auto [load, ambientTexture] = LoadDeviceTextureFromFile(loader, mat.ambient_texname);
		if (load)
			material.ambientTexture = ambientTexture;

		auto [load, albedoTexture] = LoadDeviceTextureFromFile(loader, mat.diffuse_texname);
		if (load)
			material.albedoTexture = albedoTexture;

		auto [load, specularTexture] = LoadDeviceTextureFromFile(loader, mat.specular_texname);
		if (load)
			material.specularTexture = specularTexture;

		material.ambient = XMFLOAT3(mat.ambient);
		if ((material.ambient.x == 1.f) &&
			(material.ambient.y == 1.f) &&
			(material.ambient.z == 1.f))
		{	// Fix ambient
			material.ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);
		}
		material.diffuse = XMFLOAT3(mat.diffuse);
		material.specular = XMFLOAT3(mat.specular);
		material.shininess = mat.shininess;

		return material;
	}
} // namespace h2r
