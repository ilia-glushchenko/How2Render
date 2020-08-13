#pragma once

#include "Helpers/TextureLoader.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "ThirdParty/tiny_obj_loader.h"
#include "Wrapper/Shader.hpp"
#include "Wrapper/VertexBuffer.hpp"

namespace h2r
{
	inline XMFLOAT2 LoadVec2(std::vector<tinyobj::real_t> const &attribs, int index)
	{
		XMFLOAT2 v;
		if (index < 0)
		{
			v.x = v.y = 0.f;
		}
		else
		{
			v.x = attribs[index * 2];
			v.y = attribs[index * 2 + 1];
		}
		return v;
	}

	inline XMFLOAT3 LoadVec3(std::vector<tinyobj::real_t> const &attribs, int index)
	{
		XMFLOAT3 v;
		if (index < 0)
		{
			v.x = v.y = v.z = 0.f;
		}
		else
		{
			v.x = attribs[index * 3];
			v.y = attribs[index * 3 + 1];
			v.z = attribs[index * 3 + 2];
		}
		return v;
	}

	DeviceMesh LoadObjMesh(Context const &context, tinyobj::shape_t const &shape, tinyobj::attrib_t const &attrib)
	{
		HostMesh mesh;

		const uint32_t numFaces = (uint32_t)(shape.mesh.indices.size() / 3);
		for (uint32_t faceIndex = 0; faceIndex < numFaces; ++faceIndex)
		{
			const uint32_t firstIndex = faceIndex * 3;
			const tinyobj::index_t idx[3] = {
				shape.mesh.indices[firstIndex],
				shape.mesh.indices[firstIndex + 1],
				shape.mesh.indices[firstIndex + 2]};

			XMFLOAT3 pos[3];
			for (int i = 0; i < 3; ++i)
			{
				pos[i] = LoadVec3(attrib.vertices, idx[i].vertex_index);
			}

			XMFLOAT2 texCoord[3];
			if (attrib.texcoords.empty())
			{
				for (int i = 0; i < 3; ++i)
				{

					texCoord[i] = XMFLOAT2(0.f, 0.f);
				}
			}
			else
			{
				for (int i = 0; i < 3; ++i)
				{
					texCoord[i] = LoadVec2(attrib.texcoords, idx[i].texcoord_index);
				}
			}

			bool invalidNormal = false;
			XMFLOAT3 normal[3];

			if (attrib.normals.empty())
			{
				invalidNormal = true;
			}
			else
			{
				if ((idx[0].normal_index < 0) ||
					(idx[1].normal_index < 0) ||
					(idx[2].normal_index < 0))
				{
					invalidNormal = true;
				}
				else
				{
					for (int i = 0; i < 3; ++i)
					{
						normal[i] = LoadVec3(attrib.normals, idx[i].normal_index);
					}
				}
			}

			if (invalidNormal)
			{
				XMVECTOR n = math::CalculateTriangleNormal(pos);
				for (int i = 0; i < 3; ++i)
				{
					XMStoreFloat3(&normal[i], n);
				}
			}

			for (int i = 0; i < 3; ++i)
			{
				mesh.vertices.push_back({pos[i], normal[i], texCoord[i]});
			}
		}

		return CreateDeviceMesh(context, mesh, shape.mesh.material_ids[0]);
	}

	DeviceMaterial LoadObjMaterial(tinyobj::material_t const &mat, TextureLoader &loader)
	{
		DeviceMaterial material;

		auto [loadAmbient, ambientTexture] = LoadDeviceTextureFromFile(loader, mat.ambient_texname);
		if (loadAmbient)
		{
			material.ambientTexture = ambientTexture;
		}

		auto [loadDiffuse, albedoTexture] = LoadDeviceTextureFromFile(loader, mat.diffuse_texname);
		if (loadDiffuse)
		{
			material.albedoTexture = albedoTexture;
		}

		auto [loadSpecular, specularTexture] = LoadDeviceTextureFromFile(loader, mat.specular_texname);
		if (loadSpecular)
		{
			material.specularTexture = specularTexture;
		}

		material.ambient = XMFLOAT3(mat.ambient);
		if ((material.ambient.x == 1.f) &&
			(material.ambient.y == 1.f) &&
			(material.ambient.z == 1.f))
		{ // Fix ambient
			material.ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);
		}
		material.diffuse = XMFLOAT3(mat.diffuse);
		material.specular = XMFLOAT3(mat.specular);
		material.shininess = mat.shininess;

		return material;
	}

	std::tuple<bool, DeviceModel> LoadObjModel(std::string const &fileName, TextureLoader &loader)
	{
		size_t slashPos = fileName.find_last_of("/");
		if (std::string::npos == slashPos)
		{
			slashPos = fileName.find_last_of("\\");
		}
		const std::string baseDir = (slashPos != std::string::npos) ? fileName.substr(0, slashPos) : ".";

		DeviceModel model;
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		bool load = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileName.c_str(), baseDir.c_str());

		if (!warn.empty())
		{
			printf("warning: %s\n", warn.c_str());
		}

		if (!err.empty())
		{
			printf("error: %s\n", err.c_str());
		}

		if (!load)
		{
			return {false, model};
		}

		for (auto const &shape : shapes)
		{
			DeviceMesh mesh = LoadObjMesh(loader.context, shape, attrib);
			model.meshes.push_back(mesh);
		}

		loader.baseDir = baseDir;
		for (auto const &mat : materials)
		{
			DeviceMaterial material = LoadObjMaterial(mat, loader);
			model.materials.push_back(material);
		}

		return {true, model};
	}
} // namespace h2r
