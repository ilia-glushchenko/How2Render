#pragma once

#include "Application.hpp"
#include "Math.hpp"
#include "Mesh.hpp"
#include "MipmapGenerator.hpp"
#include "RenderObject.hpp"
#include "TextureLoader.hpp"
#include "Wrapper/Context.hpp"
#include <GeometricPrimitive.h>

namespace h2r
{

	inline HostMesh GenerateSphereHostMesh(Context const &context, float radius, uint32_t tesselation)
	{
		std::vector<DirectX::GeometricPrimitive::VertexType> vertices;
		std::vector<uint16_t> indices;
		DirectX::GeometricPrimitive::CreateSphere(vertices, indices, radius * 2.f, tesselation, false);

		HostMesh sphere;
		sphere.indices.reserve(indices.size());
		sphere.vertices.reserve(vertices.size());
		for (auto i : indices)
		{
			sphere.indices.push_back(i);
		}
		for (auto &v : vertices)
		{
			sphere.vertices.push_back(Vertex{v.position, v.normal, v.textureCoordinate});
		}
		sphere.materialId = InvalidMaterialId;

		return sphere;
	}

	inline std::vector<RenderObject> GenerateSpheres(Context const &context, TextureCache &cache)
	{
		DeviceMaterial material;
		material.scalarAmbient = XMFLOAT3(1.f, 1.f, 1.f);
		material.scalarDiffuse = XMFLOAT3(1.f, 1.f, 1.f);
		material.scalarSpecular = XMFLOAT3(1.f, 1.f, 1.f);
		material.scalarShininess = 0.f;
		material.scalarAlpha = 0.5f;
		material.alphaMask = eAlphaMask::Transparent;

		RenderObject sphereBlue;
		RenderObject sphereRed;
		RenderObject sphereGreen;
		DeviceTexture::Descriptor desc;

		//Red
		{
			auto hostTexture = LoadTextureFromFile(cache,
												   "Data/Textures/sponza_fabric_diff.tga",
												   TEX_LOAD_FLAG_FLIP_VERTICALLY | TEX_LOAD_FLAG_GEN_CPU_MIPMAP,
												   DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
			if (hostTexture)
			{
				desc.hostTexture = hostTexture.value();
				auto texture = CreateDeviceTexture(context, desc);
				if (texture)
				{
					material.albedoTexture = texture.value();
				}
			}

			DeviceModel model;
			model.transparentMeshes = {CreateDeviceMesh(context, GenerateSphereHostMesh(context, 0.5f, 16))};
			model.transparentMeshes[0].materialId = 0;
			model.materials = {material};

			sphereRed = CreateRenderObject(model, {2, 0.5, -0.3f}, {0, 0, 0}, 1);
		}

		//Green
		{
			auto hostTexture = LoadTextureFromFile(cache,
												   "Data/Textures/sponza_fabric_green_diff.tga",
												   TEX_LOAD_FLAG_FLIP_VERTICALLY | TEX_LOAD_FLAG_GEN_CPU_MIPMAP,
												   DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
			if (hostTexture)
			{
				desc.hostTexture = hostTexture.value();
				auto texture = CreateDeviceTexture(context, desc);
				if (texture)
				{
					material.albedoTexture = texture.value();
				}
			}

			DeviceModel model;
			model.transparentMeshes = {CreateDeviceMesh(context, GenerateSphereHostMesh(context, 0.5f, 16))};
			model.transparentMeshes[0].materialId = 0;
			model.materials = {material};

			sphereGreen = CreateRenderObject(model, {0, 0.5, -0.3f}, {0, 0, 0}, 1);
		}

		//Blue
		{
			auto hostTexture = LoadTextureFromFile(cache,
												   "Data/Textures/sponza_fabric_blue_diff.tga",
												   TEX_LOAD_FLAG_FLIP_VERTICALLY | TEX_LOAD_FLAG_GEN_CPU_MIPMAP,
												   DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
			if (hostTexture)
			{
				desc.hostTexture = hostTexture.value();
				auto texture = CreateDeviceTexture(context, desc);
				if (texture)
				{
					material.albedoTexture = texture.value();
				}
			}

			DeviceModel model;
			model.transparentMeshes = {CreateDeviceMesh(context, GenerateSphereHostMesh(context, 0.5f, 16))};
			model.transparentMeshes[0].materialId = 0;
			model.materials = {material};

			sphereBlue = CreateRenderObject(model, {-2, 0.5f, -0.3f}, {0, 0, 0}, 1);
		}

		return {sphereBlue, sphereRed, sphereGreen};
	}

	inline RenderObject GenerateFullscreenTriangle(Context const &context)
	{
		HostMesh triangleHostMesh;
		triangleHostMesh.vertices = {
			Vertex{
				XMFLOAT3{-1.0f, 3.0f, 0.5f},
				XMFLOAT3{0.0f, 0.0f, 1.0f},
				XMFLOAT2{0.0f, -1.0f},
			},
			Vertex{
				XMFLOAT3{3.0f, -1.0f, 0.5f},
				XMFLOAT3{0.0f, 0.0f, 1.0f},
				XMFLOAT2{2.0f, 1.0},
			},
			Vertex{
				XMFLOAT3{-1.0f, -1.0f, 0.5f},
				XMFLOAT3{0.0f, 0.0f, 1.0f},
				XMFLOAT2{0.0f, 1.0f},
			},
		};
		triangleHostMesh.indices = {0, 1, 2};

		HostModel triangleHostModel;
		triangleHostModel.opaqueMeshes = {triangleHostMesh};

		TextureCache cache;
		return CreateRenderObject(CreateDeviceModel(context, cache, triangleHostModel), {}, {}, 1.f);
	}

} // namespace h2r
