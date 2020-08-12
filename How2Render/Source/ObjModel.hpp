#pragma once
#include "ThirdParty/tinyobjloader/tiny_obj_loader.h"
#include "Wrapper/VertexBuffer.hpp"
#include "Math.hpp"
#include "ObjMaterial.hpp"

namespace h2r
{
	struct ObjModel
	{
        static constexpr int InvalidMaterialId = -1;

		struct Vertex
		{
			XMFLOAT3 position;
			XMFLOAT3 normal;
			XMFLOAT2 texCoord;
			Vertex() = default;
			Vertex(XMFLOAT3 const& p, XMFLOAT3 const& n, XMFLOAT2 const& uv):
				position(p), normal(n), texCoord(uv) {}
		};

		struct Mesh
		{
			VertexBuffer vertexBuffer;
			int materialId; // -1 - Invalid
		};

		std::vector<Mesh> meshes;
		std::vector<ObjMaterial> materials;
	};

	inline XMFLOAT2 LoadVec2(std::vector<tinyobj::real_t> const& attribs, int index)
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

	inline XMFLOAT3 LoadVec3(std::vector<tinyobj::real_t> const& attribs, int index)
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

    ObjModel::Mesh LoadObjMesh(Context const& context, tinyobj::shape_t const& shape, tinyobj::attrib_t const& attrib)
    {
        std::vector<ObjModel::Vertex> vertices;

		const uint32_t numFaces = (uint32_t)(shape.mesh.indices.size() / 3);
		for (uint32_t faceIndex = 0; faceIndex < numFaces; ++faceIndex)
		{
			const uint32_t firstIndex = faceIndex * 3;
			const tinyobj::index_t idx[3] = {
				shape.mesh.indices[firstIndex],
				shape.mesh.indices[firstIndex + 1],
				shape.mesh.indices[firstIndex + 2]
			};

			XMFLOAT3 pos[3];
			for (int i = 0; i < 3; ++i)
				pos[i] = LoadVec3(attrib.vertices, idx[i].vertex_index);

			XMFLOAT2 texCoord[3];
			if (attrib.texcoords.empty())
			{
				for (int i = 0; i < 3; ++i)
					texCoord[i] = XMFLOAT2(0.f, 0.f);
			}
			else
			{
				for (int i = 0; i < 3; ++i)
					texCoord[i] = LoadVec2(attrib.texcoords, idx[i].texcoord_index);
			}

			bool invalidNormal = false;
			XMFLOAT3 normal[3];

			if (attrib.normals.empty())
				invalidNormal = true;
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
						normal[i] = LoadVec3(attrib.normals, idx[i].normal_index);
				}
			}

			if (invalidNormal)
			{
				XMVECTOR n = math::CalculateTriangleNormal(pos);
				for (int i = 0; i < 3; ++i)
					XMStoreFloat3(&normal[i], n);
			}

			for (int i = 0; i < 3; ++i)
				vertices.emplace_back(pos[i], normal[i], texCoord[i]);
		}

		ObjModel::Mesh mesh;

		mesh.vertexBuffer = CreateVertexBuffer(context, vertices);
		mesh.materialId = shape.mesh.material_ids[0]; // Use first material ID

        return mesh;
    }

	std::tuple<bool, ObjModel> LoadObjModel(std::string const& fileName, TextureLoader& loader)
	{
		size_t slashPos = fileName.find_last_of("/");
		if (std::string::npos == slashPos)
			slashPos = fileName.find_last_of("\\");
		const std::string baseDir = (slashPos != std::string::npos) ? fileName.substr(0, slashPos) : ".";

		ObjModel model;
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		bool load = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileName.c_str(), baseDir.c_str());

		if (!warn.empty())
			printf("warning: %s\n", warn.c_str());

		if (!err.empty())
			printf("error: %s\n", err.c_str());

		if (!load)
			return {false, model};

		for (auto const& shape : shapes)
		{
            ObjModel::Mesh mesh = LoadObjMesh(loader.context, shape, attrib);
			model.meshes.push_back(mesh);
		}

        loader.baseDir = baseDir;
		for (auto const& mat : materials)
		{
			ObjMaterial material = LoadObjMaterial(mat, loader);
			model.materials.push_back(material);
		}

		return {true, model};
	}

	void DrawObjModel(Context const& context, Shaders const& shaders, ObjModel const& model)
	{
		context.pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context.pImmediateContext->VSSetShader(shaders.pVertexShader, nullptr, 0);
		context.pImmediateContext->VSSetConstantBuffers(0, 1, &shaders.pConstantBuffer);

		context.pImmediateContext->PSSetShader(shaders.pPixelShader, nullptr, 0);
		context.pImmediateContext->PSSetConstantBuffers(0, 1, &shaders.pConstantBuffer);
		context.pImmediateContext->PSSetConstantBuffers(1, 1, &shaders.pMaterialConstants);
		context.pImmediateContext->PSSetSamplers(0, 1, &shaders.pTrilinearSampler);

		for (auto const& mesh : model.meshes)
		{
			ID3D11ShaderResourceView *shaderResourceViews[3];
			MaterialConstants materialConstants;

			if (mesh.materialId > ObjModel::InvalidMaterialId)
			{
				ObjMaterial const& material = model.materials[mesh.materialId];

				shaderResourceViews[0] = material.ambientTexture.shaderResourceView;
				shaderResourceViews[1] = material.albedoTexture.shaderResourceView;
				shaderResourceViews[2] = material.specularTexture.shaderResourceView;

				materialConstants.ambient = material.ambient;
				materialConstants.diffuse = material.diffuse;
				materialConstants.specular = material.specular;
				materialConstants.shininess = material.shininess;
			}
			else
			{
				for (int i = 0; i < _countof(shaderResourceViews); ++i)
					shaderResourceViews[i] = nullptr;

				materialConstants.ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);
				materialConstants.diffuse = XMFLOAT3(1.f, 1.f, 1.f);
				materialConstants.specular = XMFLOAT3(1.f, 1.f, 1.f);
				materialConstants.shininess = 0.f;
			}

			// Update material properties
			context.pImmediateContext->UpdateSubresource(
				shaders.pMaterialConstants, 0, nullptr, &materialConstants, 0, 0);
			// Set material textures
			context.pImmediateContext->PSSetShaderResources(0, _countof(shaderResourceViews), shaderResourceViews);

			constexpr uint32_t stride = sizeof(ObjModel::Vertex);
			constexpr uint32_t offset = 0;

			context.pImmediateContext->IASetVertexBuffers(0, 1, &mesh.vertexBuffer.pVertexBuffer, &stride, &offset);
			context.pImmediateContext->Draw(mesh.vertexBuffer.vertexCount, 0);
		}
	}

	void FreeObjModel(ObjModel& model)
	{
		for (auto& mesh : model.meshes)
		{
			ReleaseVertexBuffer(mesh.vertexBuffer);
		}

		model.meshes.clear();
	}
} // namespace h2r
