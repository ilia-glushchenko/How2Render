#pragma once

#include "Helpers/TextureLoader.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "ThirdParty/tiny_obj_loader.h"
#include "Wrapper/Shader.hpp"
#include "Wrapper/VertexBuffer.hpp"
#include <optional>

namespace h2r
{

	inline XMFLOAT2 TobjVertexToFloat2(std::vector<tinyobj::real_t> const &attribs, int index)
	{
		XMFLOAT2 v;
		if (index < 0)
		{
			v.x = v.y = 0.f;
		}
		else
		{
			v.x = attribs[size_t(index) * 2];
			v.y = attribs[size_t(index) * 2 + 1];
		}
		return v;
	}

	inline XMFLOAT3 TobjVertexToFloat3(std::vector<tinyobj::real_t> const &attribs, int index)
	{
		XMFLOAT3 v;
		if (index < 0)
		{
			v.x = v.y = v.z = 0.f;
		}
		else
		{
			v.x = attribs[size_t(index) * 3];
			v.y = attribs[size_t(index) * 3 + 1];
			v.z = attribs[size_t(index) * 3 + 2];
		}
		return v;
	}

	inline HostMesh TobjMeshToHostMesh(tinyobj::shape_t const &shape, tinyobj::attrib_t const &attrib)
	{
		HostMesh mesh;
		mesh.materialId = shape.mesh.material_ids[0];

		const uint32_t numFaces = (uint32_t)(shape.mesh.indices.size() / 3);
		for (uint32_t faceIndex = 0; faceIndex < numFaces; ++faceIndex)
		{
			const uint32_t firstIndex = faceIndex * 3;
			const tinyobj::index_t idx[3] = {
				shape.mesh.indices[firstIndex],
				shape.mesh.indices[size_t(firstIndex) + 1],
				shape.mesh.indices[size_t(firstIndex) + 2]};

			XMFLOAT3 pos[3];
			for (int i = 0; i < 3; ++i)
			{
				pos[i] = TobjVertexToFloat3(attrib.vertices, idx[i].vertex_index);
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
					texCoord[i] = TobjVertexToFloat2(attrib.texcoords, idx[i].texcoord_index);
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
						normal[i] = TobjVertexToFloat3(attrib.normals, idx[i].normal_index);
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

		return mesh;
	}

	inline HostMaterial TobjMaterialToHostMaterial(tinyobj::material_t const &mat, TextureCache &cache, std::filesystem::path const &modelDir)
	{
		HostMaterial material;

		auto [loadAmbient, ambientTexture] = LoadTextureFromFile(
			cache, modelDir / mat.ambient_texname, TEX_LOAD_FLAG_FLIP_VERTICALLY | TEX_LOAD_FLAG_GEN_CPU_MIPMAP);
		if (loadAmbient)
		{
			material.ambientTexture = ambientTexture;
		}
		auto [loadAlbedo, albedoTexture] = LoadTextureFromFile(
			cache, modelDir / mat.diffuse_texname, TEX_LOAD_FLAG_FLIP_VERTICALLY | TEX_LOAD_FLAG_GEN_CPU_MIPMAP);
		if (loadAlbedo)
		{
			material.albedoTexture = albedoTexture;
		}
		auto [loadSpecular, specularTexture] = LoadTextureFromFile(
			cache, modelDir / mat.specular_texname, TEX_LOAD_FLAG_FLIP_VERTICALLY | TEX_LOAD_FLAG_GEN_CPU_MIPMAP);
		if (loadSpecular)
		{
			material.specularTexture = specularTexture;
		}

		material.scalarAmbient = XMFLOAT3(mat.ambient);
		material.scalarDiffuse = XMFLOAT3(mat.diffuse);
		material.scalarSpecular = XMFLOAT3(mat.specular);
		material.scalarShininess = mat.shininess;
		material.scalarAlpha = 1.0f; // OBJ doesn't have scalar alpha value
		material.alphaMask = (mat.dissolve == 0.f) ? eAlphaMask::Opaque : eAlphaMask::Transparent;

		return material;
	}

	inline std::optional<HostModel> LoadObjModel(std::filesystem::path path, TextureCache &cache)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		wprintf(L"Loading model %s\n", path.filename().c_str());
		bool const load = tinyobj::LoadObj(
			&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), path.parent_path().string().c_str());
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
			return {};
		}

		HostModel model;
		for (auto const &tobjMaterial : materials)
		{
			model.materials.push_back(TobjMaterialToHostMaterial(tobjMaterial, cache, path.parent_path()));
		}

		// We need to separate original model based on the material basis.
		// Since we are going to draw opaque first and than transparent in the separate pass.
		// We have to split meshes that use transparent materials into one collection and meshes
		// that use opaque material into another collection.
		for (auto const &shape : shapes)
		{
			switch (model.materials[shape.mesh.material_ids[0]].alphaMask)
			{
			case eAlphaMask::Opaque:
				model.opaqueMeshes.push_back(TobjMeshToHostMesh(shape, attrib));
				break;
			case eAlphaMask::Transparent:
				model.transparentMeshes.push_back(TobjMeshToHostMesh(shape, attrib));
				break;
			default:
				printf("Host materials has invalid alpha mask value");
				assert(true);
				return {};
			}
		}

		return model;
	}

} // namespace h2r
