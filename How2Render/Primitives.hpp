#pragma once

#include "Math.hpp"
#include <vector>

struct Material
{
	XMFLOAT3 albedo;
	XMFLOAT3 emissive;
};

struct Plane
{
	Material material;
	XMVECTOR point;
	XMVECTOR normal;
};

struct Sphere
{
	Material material;
	XMVECTOR position;
	float radius;
};

struct Scene
{
	std::vector<Sphere> spheres;
	std::vector<Plane> planes;
};

struct Ray
{
	XMVECTOR origin;
	XMVECTOR direction;
};

struct RayHit
{
	XMVECTOR normal;
	XMVECTOR point;
	Material material;
};

Scene CreateSimpleScene()
{
	Scene const scene{
		{
			Sphere{
				Material{
					XMFLOAT3{0.f, 0.f, 0.f},
					XMFLOAT3{1.f, 1.f, 1.f},
				},
				XMVECTOR{0.f, 0.f, -5.f, 1.f},
				0.5f},
		},
		{
			Plane{
				//Bottom
				Material{
					XMFLOAT3{0.9f, 0.9f, 0.75f},
					XMFLOAT3{0.f, 0.f, 0.f},
				},
				XMVECTOR{0.f, -1.f, 0.f, 1.f},
				XMVECTOR{0.f, 1.f, 0.f},
			},
		},
	};

	return scene;
}

Scene CreateDefaultScene()
{
	Scene const scene{
		{Sphere{
			 Material{
				 XMFLOAT3{0.9f, 0.9f, 0.75f},
				 XMFLOAT3{0.f, 0.f, 0.f},
			 },
			 XMVECTOR{-1.5f, 0.5f, 0.f, 1.f},
			 0.5f},
		 Sphere{
			 Material{
				 XMFLOAT3{0.9f, 0.75f, 0.9f},
				 XMFLOAT3{0.f, 0.f, 0.f},
			 },
			 XMVECTOR{0.f, 0.5f, 0.f, 1.f},
			 0.5f},
		 Sphere{
			 Material{
				 XMFLOAT3{0.75f, 0.9f, 0.9f},
				 XMFLOAT3{0.f, 0.f, 0.f},
			 },
			 XMVECTOR{1.5f, 0.5f, 0.f, 1.f},
			 0.5f},
		 Sphere{
			 Material{
				 XMFLOAT3{1.f, 1.f, 1.f},
				 XMFLOAT3{1.f, 1.f, 1.f},
			 },
			 XMVECTOR{0.f, 4.f, 0.f, 1.f},
			 1.f}},
		{Plane{
			Material{
				XMFLOAT3{1.f, 1.f, 1.f},
				XMFLOAT3{0.f, 0.f, 0.f},
			},
			XMVECTOR{0.f, 0.f, 0.f, 1.f},
			XMVECTOR{0.f, 1.f, 0.f},
		}},
	};

	return scene;
}

Scene CreateCornellBox()
{
	Scene const scene{
		{Sphere{
			 Material{
				 XMFLOAT3{0.9f, 0.9f, 0.75f},
				 XMFLOAT3{0.f, 0.f, 0.f},
			 },
			 XMVECTOR{-1.5f, 0.5f, 0.f, 1.f},
			 0.5f},
		 Sphere{
			 Material{
				 XMFLOAT3{0.9f, 0.75f, 0.9f},
				 XMFLOAT3{0.f, 0.f, 0.f},
			 },
			 XMVECTOR{0.f, 0.5f, 0.f, 1.f},
			 0.5f},
		 Sphere{
			 Material{
				 XMFLOAT3{0.75f, 0.9f, 0.9f},
				 XMFLOAT3{0.f, 0.f, 0.f},
			 },
			 XMVECTOR{1.5f, 0.5f, 0.f, 1.f},
			 0.5f},
		 Sphere{
			 Material{
				 XMFLOAT3{1.f, 1.f, 1.f},
				 XMFLOAT3{1.f, 1.f, 1.f},
			 },
			 XMVECTOR{0.f, 4.f, 0.f, 1.f},
			 1.f}},
		{Plane{
			 // Back
			 Material{
				 XMFLOAT3{1.f, 1.f, 1.f},
				 XMFLOAT3{0.f, 0.f, 0.f},
			 },
			 XMVECTOR{0.f, 0.f, -2.f, 1.f},
			 XMVECTOR{0.f, 0.f, 1.f},
		 },
		 Plane{
			 //Bottom
			 Material{
				 XMFLOAT3{1.f, 1.f, 1.f},
				 XMFLOAT3{0.f, 0.f, 0.f},
			 },
			 XMVECTOR{0.f, 0.f, 0.f, 1.f},
			 XMVECTOR{0.f, 1.f, 0.f},
		 },
		 Plane{
			 //Top
			 Material{
				 XMFLOAT3{1.f, 1.f, 1.f},
				 XMFLOAT3{0.f, 0.f, 0.f},
			 },
			 XMVECTOR{0.f, 4.f, 0.f, 1.f},
			 XMVECTOR{0.f, -1.f, 0.f},
		 },
		 Plane{
			 //Left
			 Material{
				 XMFLOAT3{1.f, 0.f, 0.f},
				 XMFLOAT3{0.f, 0.f, 0.f},
			 },
			 XMVECTOR{-2.f, 0.f, 0.f, 1.f},
			 XMVECTOR{1.f, 0.f, 0.f},
		 },
		 Plane{
			 //Right
			 Material{
				 XMFLOAT3{0.f, 1.f, 0.f},
				 XMFLOAT3{0.f, 0.f, 0.f},
			 },
			 XMVECTOR{2.f, 0.f, 0.f, 1.f},
			 XMVECTOR{-1.f, 0.f, 0.f},
		 }},
	};

	return scene;
}

Scene TransformScene2ViewSpace(Scene const &scene, Camera const &camera)
{
	Scene sceneView;

	for (Sphere const &sphere : scene.spheres)
	{
		sceneView.spheres.push_back(Sphere{
			sphere.material,
			XMVector4Transform(sphere.position, camera.view),
			sphere.radius});
	}
	for (Plane const &plane : scene.planes)
	{
		sceneView.planes.push_back(Plane{
			plane.material,
			XMVector4Transform(plane.point, camera.view),
			XMVector4Transform(plane.normal, camera.view)});
	}

	return sceneView;
}