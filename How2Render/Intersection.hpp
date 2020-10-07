#pragma once

#include "Primitives.hpp"
#include <DirectXMath.h>
#include <cassert>

inline bool RaySphereIntersection(Sphere const &sphere, Ray const &ray)
{
    float const threshold = 1e-3f;
    assert(sphere.radius > threshold);

    //Let's move sphere into the ray model space
    DirectX::FXMVECTOR const raySphere = DirectX::XMVectorSubtract(sphere.position, ray.origin);

    //T center is basically a distance along ray direction
    //we need to move in order to be as close as possible to the sphere center
    float const tCenter = DirectX::XMVector3Dot(raySphere, ray.direction).m128_f32[0];

    //Let's calculate the distance from that point to the center of the sphere
    //and see if it's less than sphere radius.
    //Note: We use squares cause it's a little bit chipper to compute
    float const distanceSquare = DirectX::XMVector3Dot(raySphere, raySphere).m128_f32[0] - tCenter * tCenter;

    //Let's use some threshold value for the edge cases
    bool const hit = (tCenter > threshold) && (sphere.radius * sphere.radius - distanceSquare > threshold);

    return hit;
}