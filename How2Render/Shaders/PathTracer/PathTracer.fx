//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix View;
    uint ScreenWidth;
    uint ScreenHeight;
    uint FrameCount;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = float4(input.Pos, 1);
    output.Tex = input.Tex;

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

// Primitives
struct Ray {
    float3 origin;
    float3 direction;
};

struct Material
{
    float3 albedo;
    float3 emissive;
};

struct Sphere
{
    float3 position;
    float radius;
    Material material;
};

struct Plane
{
    float3 position;
    float3 normal;
    Material material;
};

struct Intersection
{
    bool hit;
    float t;
};

struct Manifold
{
    float3 contact_point;
    float3 contact_normal;
    Material material;
};

static const float PI = 3.14159265f;

// Random
uint wang_hash(inout uint seed)
{
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}

float random_float(inout uint state)
{
    return float(wang_hash(state)) / 4294967296.0;
}

float3 random_unit_vector(inout uint state)
{
    float z = random_float(state) * 2.0f - 1.0f;
    float a = random_float(state) * 2.0f * PI;
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return float3(x, y, z);
}

// Intersection
Intersection ray_plane_intersection(Plane plane, Ray ray)
{
    Intersection intersection;

    float threshold = 1e-3f;

    float denom = dot(ray.direction, plane.normal);
    intersection.t = dot(plane.position - ray.origin, plane.normal) / denom;
    intersection.hit = intersection.t > threshold;

    return intersection;
}

Intersection ray_sphere_intersection(Sphere sphere, Ray ray)
{
    Intersection intersection;

    float threshold = 1e-3f;

    float3 sphere_center = sphere.position - ray.origin;
    float tCenter = dot(sphere_center, ray.direction);
    float distance_square = dot(sphere_center, sphere_center) - tCenter * tCenter;

    intersection.hit = (tCenter > threshold) && (sphere.radius * sphere.radius - distance_square > threshold);
    float tDelta = intersection.hit ? sqrt(sphere.radius * sphere.radius - distance_square) : 0;
    intersection.t = tCenter - tDelta;

    return intersection;
}

float3 calculate_ray_sphere_contact_normal(float3 contact_point, float3 sphere_center)
{
    return normalize(contact_point - sphere_center);
}

float3 calculate_contact_point(float t, Ray ray)
{
    return ray.origin + ray.direction * t;
}

float4 PS(PS_INPUT input) : SV_Target
{
    uint rngState = uint(
        uint(input.Tex.x * ScreenWidth) * uint(1973)
        + uint(input.Tex.y * ScreenHeight) * uint(9277)
        + uint(FrameCount) * uint(26699)
    ) | uint(1);

    float3 leftTop = normalize(float3(-1, 1, -1));
    float3 rightBottom = normalize(float3(1, -1, -1));
    float3 direction = normalize(lerp(leftTop, rightBottom, float3(input.Tex, 0)));

    Sphere sphere[4];
    sphere[0].position = float3(0, 0, -5);
    sphere[0].radius = 0.5;
    sphere[0].material.albedo = float3(1, 0, 0);
    sphere[0].material.emissive = float3(0, 0, 0);

    sphere[1].position = float3(-1.5, 0, -5);
    sphere[1].radius = 0.5;
    sphere[1].material.albedo = float3(0, 1, 0);
    sphere[1].material.emissive = float3(0, 0, 0);

    sphere[2].position = float3(1.5, 0, -5);
    sphere[2].radius = 0.5;
    sphere[2].material.albedo = float3(0, 0, 1);
    sphere[2].material.emissive = float3(0, 0, 0);

    sphere[3].position = float3(0, 3, -5);
    sphere[3].radius = 1;
    sphere[3].material.albedo = float3(1, 1, 1);
    sphere[3].material.emissive = float3(1, 1, 1);

    Plane planes[5];
    //Bottom
    planes[0].position = float3(0, -0.5, 0);
    planes[0].normal = float3(0, 1, 0);
    planes[0].material.albedo = float3(1, 1, 1);
    planes[0].material.emissive = float3(0, 0, 0);

    //Back
    planes[1].position = float3(0, 0, -8);
    planes[1].normal = float3(0, 0, 1);
    planes[1].material.albedo = float3(1, 1, 1);
    planes[1].material.emissive = float3(0, 0, 0);

    //Top
    planes[2].position = float3(0, 3, 0);
    planes[2].normal = float3(0, -1, 0);
    planes[2].material.albedo = float3(1, 1, 1);
    planes[2].material.emissive = float3(0, 0, 0);

    //Left
    planes[3].position = float3(-2, 0, 0);
    planes[3].normal = float3(1, 0, 0);
    planes[3].material.albedo = float3(1, 0, 0);
    planes[3].material.emissive = float3(0, 0, 0);

    //Right
    planes[4].position = float3(2, 0, 0);
    planes[4].normal = float3(-1, 0, 0);
    planes[4].material.albedo = float3(0, 1, 0);
    planes[4].material.emissive = float3(0, 0, 0);

    Ray ray;
    ray.origin = float3(0, 0, 0);
    ray.direction = direction;

    //float4 out_color = float4(input.Tex, 0.0f, 1.0f);
    Manifold manifold;
    Intersection intersection;
    float3 throughput = float3(1, 1, 1);
    float3 color = float3(0, 0, 0);

    for (uint bounce_count = 0; bounce_count < 8; bounce_count++)
    {
        float t = 1e6;

        for (uint i = 0; i < 4; ++i)
        {
            Sphere sphere_view = sphere[i];
            sphere_view.position = mul(float4(sphere[i].position, 1), View).xyz;

            intersection = ray_sphere_intersection(sphere_view, ray);
            if (intersection.hit && intersection.t < t)
            {
                t = intersection.t;

                manifold.contact_point = calculate_contact_point(t, ray);
                manifold.contact_normal = calculate_ray_sphere_contact_normal(manifold.contact_point, sphere_view.position);
                manifold.material = sphere_view.material;
            }
        }

        for (uint j = 0; j < 5; ++j)
        {
            Plane plane_view = planes[j];
            plane_view.position = mul(float4(planes[j].position, 1), View).xyz;
            plane_view.normal = mul(float4(planes[j].normal, 0), View).xyz;

            intersection = ray_plane_intersection(plane_view, ray);
            if (intersection.hit && intersection.t < t)
            {
                t = intersection.t;

                manifold.contact_point = calculate_contact_point(t, ray);
                manifold.contact_normal = plane_view.normal;
                manifold.material = plane_view.material;
            }
        }

        if (t == 1e6) {
            break;
        }

        ray.origin = manifold.contact_point;
        ray.direction = normalize(manifold.contact_normal + random_unit_vector(rngState));
        color += manifold.material.emissive * throughput;
        throughput *= manifold.material.albedo;
    }

    float3 prev_color = txDiffuse.Sample(samLinear, input.Tex).rgb;
    color = lerp(prev_color, color, 1.f / float(FrameCount + 1));

    return float4(color, 1);
}
