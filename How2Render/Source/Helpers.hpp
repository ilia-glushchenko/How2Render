#pragma once

#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

namespace help
{
	inline glm::mat4 CreateCameraMatrix(glm::vec3 pos, float yaw, float pitch)
	{
		return glm::translate(pos) * glm::eulerAngleY(yaw) * glm::eulerAngleX(pitch);
	}

	inline glm::mat4 CreateViewMatrix(glm::vec3 pos, float yaw, float pitch)
	{
		return glm::eulerAngleX(-pitch) * glm::eulerAngleY(-yaw) * glm::translate(-pos);
	}
}

