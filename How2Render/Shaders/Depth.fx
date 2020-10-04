//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const float AlphaThreshold = 0.3f;

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
#ifdef ENABLE_TRANSPARENCY
Texture2D ambientTexture : register(t0);
#endif
Texture2D normalMapTexture : register(t3);

SamplerState textureSampler : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer PerInstanceCB : register(b0)
{
	matrix World;
};

cbuffer PerFrameCB : register(b2)
{
	matrix View;
	matrix Proj;
	matrix inverseView;
	matrix inverseProj;
	float4 CameraPos;
};

cbuffer InfrequentCB : register(b3)
{
	uint FinalOutputIndex;
	uint SsaoKernelSize;
	float SsaoKernelRadius;
	float SsaoBias;
	float4 SsaoKernel[64];
	uint NormalMappingEnabled;
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 WorldPos : TEXCOORD1;
};

struct PS_OUTPUT
{
	float2 Normal: SV_Target0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	matrix WVP = mul(mul(Proj, View), World);
	output.Pos = mul(WVP, float4(input.Pos, 1.f));
	output.Normal = mul(World, float4(input.Normal, 0)).xyz;
	output.Tex = input.Tex;
	output.WorldPos = mul(World, float4(input.Pos, 1.f)).xyz;

	return output;
}

//--------------------------------------------------------------------------------------
// Survey of Efficient Representations for Independent Unit Vectors
// http://jcgt.org/published/0003/02/01/
//--------------------------------------------------------------------------------------

// Returns +/- 1
float2 signNotZero(float2 v)
{
	return float2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

// Assume normalized input. Output is on [-1, 1] for each component.
float2 float32x3_to_oct(float3 v)
{
	// Project the sphere onto the octahedron, and then onto the xy plane
	float2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
	// Reflect the folds of the lower hemisphere over the diagonals
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * signNotZero(p)) : p;
}

//--------------------------------------------------------------------------------------
// Christian Schuler, "Normal Mapping without Precomputed Tangents", ShaderX 5, Chapter 2.6.
// http://www.thetenthplanet.de/archives/1180
//--------------------------------------------------------------------------------------
float3x3 cotangentFrame(float3 N, float3 p, float2 uv)
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
// Pixel Shader
//--------------------------------------------------------------------------------------

PS_OUTPUT PS(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;

#ifdef ENABLE_TRANSPARENCY
	if (ambientTexture.Sample(textureSampler, input.Tex).a < AlphaThreshold) {
		discard;
	}
#endif

	if (NormalMappingEnabled)
	{
		// Compute per-pixel cotangent frame
		float3 normal = normalize(input.Normal);
		float3x3 TBN = cotangentFrame(normal, input.WorldPos, input.Tex);

		// In terms of normal maps, the difference result in how the green channel of a RGB texture should be interpreted.
		// OpenGL expects the first pixel to be at the bottom while DirectX expects it to be at the top
		// https://docs.substance3d.com/bake/what-is-the-difference-between-the-opengl-and-directx-normal-format-182256965.html
		float3 micronormal = normalMapTexture.Sample(textureSampler, input.Tex).xyz;
		micronormal.y = 1. - micronormal.y;

		// Transform normal from texture space to object space
		float3 n = mul(normalize(micronormal * 2. - 1.), TBN);

		output.Normal = float32x3_to_oct(n) * 0.5f + 0.5;
	}
	else
	{
		output.Normal = float32x3_to_oct(input.Normal) * 0.5f + 0.5;
	}

	return output;
}
