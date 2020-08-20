#pragma once

#include "Primitives.hpp"
#include <DirectXMath.h>
#include <cassert>
#include <cmath>
#include <tuple>

inline std::tuple<bool, float> CalculateRaySphereIntersection(Sphere const &sphere, Ray const &ray, float threshold = 1e-3f)
{
	assert(sphere.radius > threshold);

	//Let's move sphere into the ray model space
	FXMVECTOR const raySphere = XMVectorSubtract(sphere.position, ray.origin);

	//T center is basically a distance along ray direction
	//we need to move in order to be as close as possible to the sphere center
	float const tCenter = XMVector3Dot(raySphere, ray.direction).m128_f32[0];

	//Let's calculate the distance from that point to the center of the sphere
	//and see if it's less than sphere radius.
	//Note: We use squares cause it's a little bit chipper to compute
	float const distanceSquare = XMVector3Dot(raySphere, raySphere).m128_f32[0] - tCenter * tCenter;

	//Let's use some threshold value for the edge cases
	bool const hit = (tCenter > threshold) && (sphere.radius * sphere.radius - distanceSquare > threshold);

	float const tDelta = hit ? std::sqrtf(std::pow(sphere.radius, 2) - distanceSquare) : 0;

	return {hit, tCenter - tDelta};
}

inline XMVECTOR CalculateContactPoint(float t, Ray const &ray)
{
	return XMVectorAdd(ray.origin, XMVectorScale(ray.direction, t));
}

inline XMVECTOR CalculateRaySphereContactNormal(XMVECTOR contactPoint, XMVECTOR sphereCenter)
{
	return XMVector3Normalize(XMVectorSubtract(contactPoint, sphereCenter));
}

inline std::tuple<bool, float> CalculateRayPlaneIntersection(
	Plane const &plane, Ray const &ray, float threshold = 1e-3f)
{
	float const denom = XMVector3Dot(ray.direction, plane.normal).m128_f32[0];
	float const t = XMVector3Dot(XMVectorSubtract(plane.point, ray.origin), plane.normal).m128_f32[0] / denom;

	return {t > threshold, t};
}

inline std::tuple<bool, RayHit> CalculateContact(Ray const &ray, Scene const &scene)
{
	RayHit rayhit = {};

	float intersectionDepth = FLT_MAX;
	uint32_t sphereIndex = UINT32_MAX;
	uint32_t planeIndex = UINT32_MAX;

	// Then let's check if we hit the sphere
	for (uint32_t i = 0; i < scene.spheres.size(); ++i)
	{
		if (auto [hit, t] = CalculateRaySphereIntersection(scene.spheres[i], ray);
			hit && t < intersectionDepth)
		{
			intersectionDepth = t;
			sphereIndex = i;
		}
	}

	// Let's check if we hit the plane
	for (uint32_t i = 0; i < scene.planes.size(); ++i)
	{
		if (auto [hit, t] = CalculateRayPlaneIntersection(scene.planes[i], ray);
			hit && t < intersectionDepth)
		{
			intersectionDepth = t;
			planeIndex = i;
		}
	}

	if (sphereIndex != UINT32_MAX && planeIndex == UINT32_MAX)
	{
		rayhit.point = CalculateContactPoint(intersectionDepth, ray);
		rayhit.normal = CalculateRaySphereContactNormal(rayhit.point, scene.spheres[sphereIndex].position);
		rayhit.material = scene.spheres[sphereIndex].material;
	}
	else if (planeIndex != UINT32_MAX)
	{
		rayhit.point = CalculateContactPoint(intersectionDepth, ray);
		rayhit.normal = scene.planes[planeIndex].normal;
		rayhit.material = scene.planes[planeIndex].material;
	}

	return {intersectionDepth != FLT_MAX, rayhit};
}
