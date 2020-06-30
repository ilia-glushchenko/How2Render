#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Material
{
	glm::vec3 albedo;
	glm::vec3 emissive;
};

struct Sphere
{
	Material material;
	glm::vec3 position;
	float radius;
};

struct Plane
{
	Material material;
	glm::vec3 point;
	glm::vec3 normal;
};

struct Scene
{
	std::vector<Sphere> spheres;
	std::vector<Plane> planes;
};

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

struct RayHit
{
	glm::vec3 normal;
	glm::vec3 point;
	Material material;
};

Scene create_default_scene()
{
	Scene const scene{
		{Sphere{
			 Material{
				 glm::vec3{0.9, 0.9, 0.75},
				 glm::vec3{0, 0, 0},
			 },
			 glm::vec3{-1.5, 0.5, 0},
			 0.5},
		 Sphere{
			 Material{
				 glm::vec3{0.9, 0.75, 0.9},
				 glm::vec3{0, 0, 0},
			 },
			 glm::vec3{0, 0.5, 0},
			 0.5},
		 Sphere{
			 Material{
				 glm::vec3{0.75, 0.9, 0.9},
				 glm::vec3{0, 0, 0},
			 },
			 glm::vec3{1.5, 0.5, 0},
			 0.5},
		 Sphere{
			 Material{
				 glm::vec3{1, 1, 1},
				 glm::vec3{1, 1, 1},
			 },
			 glm::vec3{0, 4, 0},
			 1}},
		//{Plane{
		//	// Back
		//	Material{
		//		glm::vec3{1, 1, 1},
		//		glm::vec3{0, 0, 0},
		//	},
		//	glm::vec3{0, 0, -2},
		//	glm::vec3{0, 0, 1},
		//},
		{Plane{
			//Bottom
			Material{
				glm::vec3{1, 1, 1},
				glm::vec3{0, 0, 0},
			},
			glm::vec3{0, 0, 0},
			glm::vec3{0, 1, 0},
		}},
		//Plane{
		// //Top
		// Material{
		//	 glm::vec3{1, 1, 1},
		//	 glm::vec3{0, 0, 0},
		// },
		// glm::vec3{0, 4, 0},
		// glm::vec3{0, -1, 0},
		//},
		//Plane{
		// //Left
		// Material{
		//	 glm::vec3{1, 0, 0},
		//	 glm::vec3{0, 0, 0},
		// },
		// glm::vec3{-2, 0, 0},
		// glm::vec3{1, 0, 0},
		//},
		//Plane{
		// //Right
		// Material{
		//	 glm::vec3{0, 1, 0},
		//	 glm::vec3{0, 0, 0},
		// },
		// glm::vec3{2, 0, 0},
		// glm::vec3{-1, 0, 0},
		//}},
	};

	return scene;
}