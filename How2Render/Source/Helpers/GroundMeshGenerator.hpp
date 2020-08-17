#pragma once

#include "Wrapper/Context.hpp"
#include "Mesh.hpp"
#include "RenderObject.hpp"
#include <GeometricPrimitive.h>

namespace h2r
{

	inline HostMesh GenerateGroundHostMesh(Context const& context, float width, float depth)
	{
		const XMFLOAT3 size(width, 0.1f, depth);
		std::vector<DirectX::GeometricPrimitive::VertexType> vertices;
		std::vector<uint16_t> indices;
		DirectX::GeometricPrimitive::CreateBox(vertices, indices, size, false);

		HostMesh ground;
		ground.indices.reserve(indices.size());
		ground.vertices.reserve(vertices.size());
		for (auto i : indices) {
			ground.indices.push_back(i);
		}
		for (auto v : vertices) {
			ground.vertices.push_back(Vertex{ v.position, v.normal, v.textureCoordinate });
		}

		return ground;
	}

	inline RenderObject GenerateGroundRenderObject(TextureLoader& loader)
	{
		auto& context = loader.context;
		auto [loadResult, hostTexture] = LoadHostTextureFromFile(loader, "Textures/default.bmp");
		assert(loadResult);

		auto [textureResult, deviceTexture] = CreateTexture(context, hostTexture);
		assert(textureResult);
		CleanupHostTexture(hostTexture);

		DeviceMaterial material;
		material.ambientTexture = deviceTexture;
		material.albedoTexture = deviceTexture;

		constexpr int materialId = 0;
		DeviceMesh groundMesh = CreateDeviceMesh(context, GenerateGroundHostMesh(context, 300.f, 300.f), materialId);

		DeviceModel model;
		model.meshes.push_back(groundMesh);
		model.materials.push_back(material);

		RenderObject object;

		object.model = model;
		object.world = XMMatrixIdentity();

		return object;
	}

} // namespace h2r
