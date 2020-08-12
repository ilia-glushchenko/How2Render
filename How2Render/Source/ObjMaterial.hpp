#pragma once
#include "ThirdParty/tinyobjloader/tiny_obj_loader.h"
#include "TextureCache.hpp"
#include "Helpers/TextureLoader.hpp"
#include "Helpers/MipmapGenerator.hpp"

#include <cstdio>

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

	std::tuple<bool, DeviceTexture> LoadTexture(Context const& context, std::string const& fileName,
		std::string const& baseDir, TextureCache& cache)
	{
		if (fileName.empty())
			return {false, DeviceTexture{}};

		std::string const filePath = baseDir + "\\" + fileName;
		auto [found, deviceTexture] = FindCachedTexture(cache, filePath);
		if (found)
			return {true, deviceTexture};

		printf("Load texture %s\n", filePath.c_str());

		auto [loadResult, hostTexture] = LoadHostTextureFromFile(filePath, true);
		if (!loadResult)
			return {false, DeviceTexture{}};

		GenerateMipmap(hostTexture);

		auto [textureResult, deviceTexture] = CreateTexture(context, hostTexture);
		if (!textureResult)
			return {false, DeviceTexture{}};

		CleanupHostTexture(hostTexture);
		CacheTexture(cache, filePath, deviceTexture);
		return {true, deviceTexture};
	}

	ObjMaterial LoadObjMaterial(Context const& context, tinyobj::material_t const& mat,
		std::string const& baseDir, TextureCache& cache)
	{
		ObjMaterial material;

		auto [load, ambientTexture] = LoadTexture(context, mat.ambient_texname, baseDir, cache);
		if (load)
			material.ambientTexture = ambientTexture;

		auto [load, albedoTexture] = LoadTexture(context, mat.diffuse_texname, baseDir, cache);
		if (load)
			material.albedoTexture = albedoTexture;

		auto [load, specularTexture] = LoadTexture(context, mat.specular_texname, baseDir, cache);
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
