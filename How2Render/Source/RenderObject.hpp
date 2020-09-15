#pragma once

#include "Helpers/ModelLoader.hpp"
#include "Helpers/TextureCache.hpp"
#include "Math.hpp"
#include "Wrapper/Context.hpp"

namespace h2r
{
	struct Transform
	{
		XMVECTOR position = {};
		XMVECTOR orientation = {};
		XMVECTOR scale = {1.f, 1.f, 1.f};
		XMMATRIX world = {};
	};

	struct RenderObject
	{
		DeviceModel model;
		Transform transform;
	};

	struct RenderObjectStorage
	{
		std::vector<RenderObject> opaque;
		std::vector<RenderObject> translucent;
	};

	inline Transform CreateTransform(XMFLOAT3 position, XMFLOAT3 orientation, float scale)
	{
		Transform transform;

		transform.position = {position.x, position.y, position.z};
		transform.orientation = {orientation.x, orientation.y, orientation.z};
		transform.scale = {scale, scale, scale};
		transform.world = XMMatrixScaling(scale, scale, scale) * XMMatrixRotationRollPitchYaw(orientation.x, orientation.y, orientation.z) * XMMatrixTranslation(position.x, position.y, position.z);

		return transform;
	}

	inline RenderObject CreateRenderObject(DeviceModel deviceModel, XMFLOAT3 position, XMFLOAT3 orientation, float scale)
	{
		RenderObject object;

		object.model = deviceModel;
		object.transform = CreateTransform(position, orientation, scale);

		return object;
	}

	inline void CleanupRenderObject(RenderObject &renderObject)
	{
		CleanupDeviceModel(renderObject.model);
	}

} // namespace h2r
