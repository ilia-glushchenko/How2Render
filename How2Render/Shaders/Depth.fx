#include "CBuffers.fx"
#include "Helpers.fx"

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
#ifdef ENABLE_TRANSPARENCY
Texture2D ambientTexture : register(t0);
#endif
Texture2D normalMapTexture : register(t3);

SamplerState textureSampler : register(s0);

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

	matrix WVP = mul(mul(Camera.Proj, Camera.View), Transform.World);
	output.Pos = mul(WVP, float4(input.Pos, 1.f));
	output.Normal = mul(Transform.World, float4(input.Normal, 0)).xyz;
	output.Tex = input.Tex;
	output.WorldPos = mul(Transform.World, float4(input.Pos, 1.f)).xyz;

	return output;
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

	float3 normal = normalize(input.Normal);
	if (Debug.NormalMappingEnabled && Material.NormalMapAvailabled)
	{
		// Compute per-pixel cotangent frame
		float3x3 TBN = CotangentFrame(normal, input.WorldPos, input.Tex);

		// In terms of normal maps, the difference result in how the green channel of a RGB texture should be interpreted.
		// OpenGL expects the first pixel to be at the bottom while DirectX expects it to be at the top
		// https://docs.substance3d.com/bake/what-is-the-difference-between-the-opengl-and-directx-normal-format-182256965.html
		float3 micronormal = normalMapTexture.Sample(textureSampler, input.Tex).xyz;
		micronormal.y = 1. - micronormal.y;

		// Transform normal from texture space to object space
		float3 n = mul(normalize(micronormal * 2. - 1.), TBN);

		output.Normal = Float32x3ToOct(n) * 0.5f + 0.5;
	}
	else
	{
		output.Normal = Float32x3ToOct(normal) * 0.5f + 0.5;
	}

	return output;
}
