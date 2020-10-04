//--------------------------------------------------------------------------------------
// Christian Schuler, "Normal Mapping without Precomputed Tangents", ShaderX 5, Chapter 2.6.
// http://www.thetenthplanet.de/archives/1180
//--------------------------------------------------------------------------------------
float3x3 CotangentFrame(float3 N, float3 p, float2 uv)
{
    float3 dp1 = ddx(p);
    float3 dp2 = ddy(p);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);

    float3 dp2perp = cross(dp2, N);
    float3 dp1perp = cross(N, dp1);
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = 1. / sqrt(max(dot(T, T), dot(B, B)));
    return float3x3(T * invmax, B * invmax, N);
}

//--------------------------------------------------------------------------------------
// Survey of Efficient Representations for Independent Unit Vectors
// http://jcgt.org/published/0003/02/01/
//--------------------------------------------------------------------------------------
// Returns +/- 1
float2 SignNotZero(float2 v)
{
    return float2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

// Assume normalized input. Output is on [-1, 1] for each component.
float2 Float32x3ToOct(float3 v)
{
    // Project the sphere onto the octahedron, and then onto the xy plane
    float2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
    // Reflect the folds of the lower hemisphere over the diagonals
    return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * SignNotZero(p)) : p;
}

float3 OctToFloat32x3(float2 e)
{
    float3 v = float3(e.xy, 1.0 - abs(e.x) - abs(e.y));
    if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * SignNotZero(v.xy);
    return normalize(v);
}

//--------------------------------------------------------------------------------------
// Position reconstruction from Depth buffer
//--------------------------------------------------------------------------------------
float3 ViewPosFromDepth(float depth, float2 TexCoord, matrix inverseProj)
{
    float4 clipSpacePosition = float4((TexCoord * 2.0 - 1.0) * float2(1, -1), depth, 1.0);
    float4 viewSpacePosition = mul(inverseProj, clipSpacePosition);
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}

float3 WorldPosFromDepth(float depth, float2 TexCoord, matrix inverseProj, matrix inverseView)
{
    float4 clipSpacePosition = float4((TexCoord * 2.0 - 1.0) * float2(1, -1), depth, 1.0);
    float4 viewSpacePosition = mul(inverseProj, clipSpacePosition);
    viewSpacePosition /= viewSpacePosition.w;
    float4 worldSpacePosition = mul(inverseView, viewSpacePosition);

    return worldSpacePosition.xyz;
}

//--------------------------------------------------------------------------------------
// Rotation
//--------------------------------------------------------------------------------------

float2 RotateVector2D(float2 vec, float sinX, float cosX)
{
    return float2(dot(vec, float2(cosX, -sinX)),
                  dot(vec, float2(sinX,  cosX)));
}