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
		auto [loadResult, hostTexture] = LoadHostTextureFromFile(loader, "Data/Textures/earth.bmp");
		assert(loadResult);
		GenerateMipmap(hostTexture);

		auto [textureResult, deviceTexture] = CreateTexture(context, hostTexture);
		assert(textureResult);
		CleanupHostTexture(hostTexture);

		DeviceMaterial material;
		material.albedoTexture = deviceTexture;
		DeviceMesh sphereMesh = CreateDeviceMesh(context, GenerateSphereHostMesh(context, 10.f, 64), 0);

		DeviceModel model;
		model.meshes.push_back(sphereMesh);
		model.materials.push_back(material);

		RenderObject object;

		object.model = model;
		object.world = XMMatrixRotationY(XMConvertToRadians(20.f));

		return object;
	}

} // namespace h2r
