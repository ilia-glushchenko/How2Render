#pragma once

#include <glm\glm.hpp>

inline std::tuple<bool, float> ray_sphere_intersection_factor(
    glm::vec3 sphere_center, float sphere_radius, Ray const& ray, float threshold = 1e-3f)
{
    assert(sphere_radius > threshold);

    sphere_center -= ray.origin;
    float const tCenter = glm::dot(sphere_center, ray.direction);
    float const distance_square = glm::dot(sphere_center, sphere_center) - tCenter * tCenter;
    bool const hit = (tCenter > threshold) && (sphere_radius * sphere_radius - distance_square > threshold);

    float const tDelta = hit ? std::sqrtf(sphere_radius * sphere_radius - distance_square) : 0;

    return { hit, tCenter - tDelta };
}

inline glm::vec3 calculate_ray_sphere_closest_contact_point(float t, Ray const& ray)
{
    return ray.origin + ray.direction * t;
}

inline glm::vec3 calculate_ray_sphere_contact_normal(glm::vec3 contact_point, glm::vec3 sphere_center)
{
    return glm::normalize(contact_point - sphere_center);
}

inline std::tuple<bool, float> ray_plane_intersection(
    glm::vec3 plane_point, glm::vec3 plane_normal, Ray const& ray, float threshold = 1e-3f)
{
    float const denom = glm::dot(ray.direction, plane_normal) ;
    float const t = glm::dot(plane_point - ray.origin, plane_normal) / denom;

    return { t > threshold, t };
}
