#pragma once

#include "Wrapper/Context.hpp"
#include "TextureLoader.hpp"
#include "MipmapGenerator.hpp"
#include "Mesh.hpp"
#include "RenderObject.hpp"
#include <GeometricPrimitive.h>

namespace h2r
{

	inline HostMesh GenerateSphereHostMesh(Context const& context, float radius, uint32_t tesselation)
	{
		std::vector<DirectX::GeometricPrimitive::VertexType> vertices;
		std::vector<uint16_t> indices;
		DirectX::GeometricPrimitive::CreateSphere(vertices, indices, radius * 2.f, tesselation, false);

		HostMesh sphere;
		sphere.indices.reserve(indices.size());
		sphere.vertices.reserve(vertices.size());
		for (auto i : indices) {
			sphere.indices.push_back(i);
		}
		for (auto v : vertices) {
			sphere.vertices.push_back(Vertex{ v.position, v.normal, v.textureCoordinate });
		}

		return sphere;
	}

	inline RenderObject GenerateSphereRenderObject(TextureLoader& loader)
	{
		auto& context = loader.context;
		auto [loadResult, hostTexture] = LoadHostTextureFromFile(loader, "Textures/stones.jpg");
		assert(loadResult);

		auto [textureResult, deviceTexture] = CreateTexture(context, hostTexture);
		assert(textureResult);
		CleanupHostTexture(hostTexture);

		DeviceMaterial material;
		material.ambientTexture = deviceTexture;
		material.albedoTexture = deviceTexture;

		constexpr float radius = 10.f;
		constexpr int materialId = 0;

		DeviceMesh sphereMesh = CreateDeviceMesh(context, GenerateSphereHostMesh(context, radius, 32), materialId);

		DeviceModel model;
		model.meshes.push_back(sphereMesh);
		model.materials.push_back(material);

		RenderObject object;

		object.model = model;
		XMMATRIX rotation = XMMatrixRotationY(XMConvertToRadians(90.f));
		XMMATRIX translation = XMMatrixTranslation(0.f, radius, 0.f);
		object.world = XMMatrixMultiply(rotation, translation);

		return object;
	}

} // namespace h2r
