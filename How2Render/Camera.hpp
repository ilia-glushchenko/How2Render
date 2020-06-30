#pragma once

#include <glm\glm.hpp>

struct Camera
{
	glm::mat4 view;
	glm::mat4 model;

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;

	float yaw;
	float pitch;

	float speed;
	bool capture_mouse;
};

Camera create_default_camera()
{
	return Camera{
		glm::mat4(1), //view
		glm::mat4(1), //model

		glm::vec3{0, 2, 3},	 //position
		glm::vec3{0, 0, -1}, //forward
		glm::vec3{0, 1, 0},	 //up

		0, //yaw
		0, //pitch

		0.1f, //speed
		false //capture mouse
	};
}

glm::mat4 create_camera_mat(glm::vec3 pos, float yaw, float pitch)
{
	return glm::translate(pos) * glm::eulerAngleY(yaw) * glm::eulerAngleX(pitch);
}

glm::mat4 create_view_mat(glm::vec3 pos, float yaw, float pitch)
{
	return glm::eulerAngleX(-pitch) * glm::eulerAngleY(-yaw) * glm::translate(-pos);
}
