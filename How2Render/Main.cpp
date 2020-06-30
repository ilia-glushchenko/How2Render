#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SIMD_AVX2
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <SDL.h>
#include <stdio.h>

#include "Window.hpp"
#include "Camera.hpp"
#include "Primitives.hpp"
#include "Intersection.hpp"
#include "Input.hpp"
#include "Random.hpp"

bool update_camera(InputEvents &input_events, Camera &camera, Window const &window)
{
	bool moved = false;

	//Update relative mouse mode
	if (input_events.keys[SDL_SCANCODE_F] == eKeyState::Press)
	{
		camera.capture_mouse = !camera.capture_mouse;
		SDL_ShowCursor(!camera.capture_mouse);
	}

	glm::vec4 const world_forward = glm::vec4(0., 0., -1., 0.);
	glm::vec4 const world_right = glm::vec4(1., 0., 0., 0.);

	glm::vec4 forward = glm::vec4(0);
	glm::vec4 right = glm::vec4(0);
	glm::mat4 camera_matrix = create_camera_mat(camera.position, camera.yaw, camera.pitch);

	if (is_key_down(input_events, SDL_SCANCODE_W))
	{
		forward = camera_matrix * world_forward * camera.speed;
		moved = true;
	}
	if (is_key_down(input_events, SDL_SCANCODE_S))
	{
		forward = camera_matrix * -world_forward * camera.speed;
		moved = true;
	}
	if (is_key_down(input_events, SDL_SCANCODE_A))
	{
		right = camera_matrix * -world_right * camera.speed;
		moved = true;
	}
	if (is_key_down(input_events, SDL_SCANCODE_D))
	{
		right = camera_matrix * world_right * camera.speed;
		moved = true;
	}
	if (is_key_down(input_events, SDL_SCANCODE_Q))
	{
		camera.position.y -= camera.speed;
		moved = true;
	}
	if (is_key_down(input_events, SDL_SCANCODE_E))
	{
		camera.position.y += camera.speed;
		moved = true;
	}

	glm::ivec2 const win_size = get_window_size(window);
	if (camera.capture_mouse)
	{
		glm::ivec2 const window_center = win_size / 2;
		glm::vec2 const mouse_dist = window_center - input_events.mouse;
		SDL_WarpMouseInWindow(window.window, window_center.x, window_center.y);

		camera.pitch += mouse_dist.y / win_size.x;
		camera.yaw += mouse_dist.x / win_size.y;

		if (glm::length2(mouse_dist) > 0)
		{
			moved = true;
		}
	}

	camera.position.x += forward.x + right.x;
	camera.position.y += forward.y + right.y;
	camera.position.z += forward.z + right.z;
	camera.view = create_view_mat(camera.position, camera.yaw, camera.pitch);

	return moved;
}

glm::vec2 calculate_ndc(glm::uvec2 frag_coord, glm::vec2 win_size)
{
	return glm::vec2(frag_coord) / (win_size - 1.f) * 2.f - 1.f;
}

Ray calculate_sample_ray(glm::vec2 ndc)
{
	static glm::vec3 const bottom_right_dir = glm::normalize(glm::vec3{1, -1, -1});
	static glm::vec3 const top_left_dir = glm::normalize(glm::vec3{-1, 1, -1});
	glm::vec3 const sample_dir = glm::normalize(
		glm::mix(top_left_dir, bottom_right_dir, glm::vec3((ndc + 1.f) * 0.5f, 0)));

	return Ray{
		glm::vec3{0},
		sample_dir,
	};
}

void write_pixel(
	Window const &window,
	glm::uvec2 frag_coord,
	glm::vec3 color,
	uint32_t count)
{
	glm::ivec2 const win_size = get_window_size(window);

	color = glm::clamp(color, {0, 0, 0}, {1, 1, 1});

	uint32_t &pixel = ((uint32_t *)window.surface->pixels)[frag_coord.x + frag_coord.y * win_size.y];

	glm::u8vec3 prev_pixel_color{};
	SDL_GetRGB(pixel, window.surface->format, &prev_pixel_color.r, &prev_pixel_color.g, &prev_pixel_color.b);
	glm::vec3 prev_color = glm::pow(glm::vec3(prev_pixel_color) / 255.f, {2.2, 2.2, 2.2});

	color = glm::pow(glm::mix(prev_color, color, 1.0f / float(count + 1)), {0.45, 0.45, 0.45});
	glm::u8vec3 pixel_color = color * 255.f;
	pixel = SDL_MapRGB(window.surface->format, pixel_color.r, pixel_color.g, pixel_color.b);
}

std::tuple<bool, RayHit> calculate_contact(
	Ray const &ray,
	Scene const &scene,
	Camera const &camera,
	glm::uvec2 frag_coord)
{
	RayHit rayhit{
		glm::vec3(0),
		glm::vec3(0),
		glm::vec3(0),
		glm::vec3(0),
	};

	float intersection_depth = FLT_MAX;
	uint32_t sphere_index = UINT32_MAX;
	uint32_t plane_index = UINT32_MAX;

	// Then let's check if we hit the sphere
	for (uint32_t i = 0; i < scene.spheres.size(); ++i)
	{
		if (auto [hit, t] = ray_sphere_intersection_factor(scene.spheres[i].position, scene.spheres[i].radius, ray);
			hit && t < intersection_depth)
		{
			intersection_depth = t;
			sphere_index = i;
		}
	}

	// Let's check if we hit the plane
	for (uint32_t i = 0; i < scene.planes.size(); ++i)
	{
		if (auto [hit, t] = ray_plane_intersection(scene.planes[i].point, scene.planes[i].normal, ray);
			hit && t < intersection_depth)
		{
			intersection_depth = t;
			plane_index = i;
		}
	}

	if (sphere_index != UINT32_MAX && plane_index == UINT32_MAX)
	{
		rayhit.point = calculate_ray_sphere_closest_contact_point(intersection_depth, ray);
		rayhit.normal = calculate_ray_sphere_contact_normal(rayhit.point, scene.spheres[sphere_index].position);
		rayhit.material = scene.spheres[sphere_index].material;
	}
	else if (plane_index != UINT32_MAX)
	{
		rayhit.point = ray.origin + ray.direction * intersection_depth;
		rayhit.normal = scene.planes[plane_index].normal;
		rayhit.material = scene.planes[plane_index].material;
	}

	return { intersection_depth != FLT_MAX, rayhit};
}

void sample_fragment(
	glm::uvec2 frag_coord,
	Window const &window,
	Camera const &camera,
	Scene const &scene_view,
	uint32_t frame_count)
{
	glm::vec2 const win_size = get_window_size(window);
	glm::vec3 color = {0, 0, 0};
	glm::vec3 throughput = {1, 1, 1};

	glm::vec2 const ndc = calculate_ndc(frag_coord, win_size);
	Ray ray_view = calculate_sample_ray(ndc);

	for (uint32_t bounce_index = 0; bounce_index < 4; ++bounce_index)
	{
		auto [hit, hit_data] = calculate_contact(ray_view, scene_view, camera, frag_coord);
		if (!hit)
		{
			break;
		}

		color += hit_data.material.emissive * throughput;
		throughput *= hit_data.material.albedo;

		ray_view = Ray{
			hit_data.point,
			glm::normalize(hit_data.normal + generate_normal_dist_inside_sphere_vector())};
	}

	write_pixel(window, frag_coord, color, frame_count);
}

void main_loop(Window const &window)
{
	Scene const scene = create_default_scene();

	Camera camera = create_default_camera();
	InputEvents input_events = create_defalt_input_events();
	uint32_t frame_count = 0;

	while (!input_events.quit)
	{
		update_input(input_events);
		if (update_camera(input_events, camera, window))
			frame_count = 0;

		Scene scene_view;
		for (Sphere const &sphere : scene.spheres)
		{
			scene_view.spheres.push_back(Sphere{
				sphere.material,
				camera.view * glm::vec4(sphere.position, 1),
				sphere.radius,
			});
		}
		for (Plane const &plane : scene.planes)
		{
			scene_view.planes.push_back(Plane{
				plane.material,
				camera.view * glm::vec4(plane.point, 1),
				camera.view * glm::vec4(plane.normal, 0),
			});
		}

		glm::ivec2 const win_size = get_window_size(window);
		glm::ivec2 frag_coors = glm::ivec2(0, 0);

		for (frag_coors.y = 0; frag_coors.y < win_size.y; ++frag_coors.y)
		{
			for (frag_coors.x = 0; frag_coors.x < win_size.x; ++frag_coors.x)
			{
				sample_fragment(
					frag_coors,
					window,
					camera,
					scene_view,
					frame_count);
			}
		}

		SDL_UpdateWindowSurface(window.window);
		frame_count++;
	}
}

int main(int argc, char *args[])
{
	Window window = create_window(640, 640);

	main_loop(window);

	return 0;
}