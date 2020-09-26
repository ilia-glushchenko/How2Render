#pragma once

#include "Material.hpp"
#include "Mesh.hpp"

namespace h2r
{

	struct HostModel
	{
		std::vector<HostMesh> opaqueMeshes;
		std::vector<HostMesh> transparentMeshes;
		std::vector<HostMaterial> materials;
	};

	struct DeviceModel
	{
		std::vector<DeviceMesh> opaqueMeshes;
		std::vector<DeviceMesh> transparentMeshes;
		std::vector<DeviceMaterial> materials;
	};

	inline DeviceModel CreateDeviceModel(Context const &context, TextureCache &cache, HostModel const &hostModel)
	{
		DeviceModel deviceModel;

		for (auto const &hostMesh : hostModel.opaqueMeshes)
		{
			deviceModel.opaqueMeshes.push_back(CreateDeviceMesh(context, hostMesh));
		}
		for (auto const &hostMesh : hostModel.transparentMeshes)
		{
			deviceModel.transparentMeshes.push_back(CreateDeviceMesh(context, hostMesh));
		}
		for (auto const &hostMaterail : hostModel.materials)
		{
			deviceModel.materials.push_back(CreateDeviceMaterial(context, cache, hostMaterail).value());
		}

		return deviceModel;
	}

	inline void CleanupDeviceModel(DeviceModel &model)
	{
		for (auto &mesh : model.opaqueMeshes)
		{
			CleanupDeviceMesh(mesh);
		}
		model.opaqueMeshes.clear();

		for (auto &mesh : model.transparentMeshes)
		{
			CleanupDeviceMesh(mesh);
		}
		model.transparentMeshes.clear();
	}

} // namespace h2r
