#include "CBuffers.fx"
#include "Helpers.fx"

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
Texture2D ambientTexture : register(t0);
Texture2D albedoTexture : register(t1);
Texture2D specularTexture : register(t2);

SamplerState anisotropicSampler : register(s0);

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
	float2 Tex : TEXCOORD0;
	float3 WorldPos : POSITION0;
};

struct PS_OUTPUT
{
	float2 Ambient: SV_Target0;
	float4 Diffuse: SV_Target1;
	float4 Specular: SV_Target2;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	matrix MVP = mul(mul(Camera.Proj, Camera.View), Transform.World);
	output.Pos = mul(MVP, float4(input.Pos, 1.f));
	output.Tex = input.Tex;
	output.WorldPos = mul(Transform.World, float4(input.Pos, 1)).xyz;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

PS_OUTPUT PS(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;

	float3 ambient = ambientTexture.Sample(anisotropicSampler, input.Tex).rgb * Material.Ambient;
	float4 albedo = albedoTexture.Sample(anisotropicSampler, input.Tex).rgba;
	float3 specular = specularTexture.Sample(anisotropicSampler, input.Tex).rgb * Material.Specular;

	// GBuffer Layout
	//     8        |       8       |       8       |      8
	// Ambient.R    | Ambient.G     |               |           | RG8_UNORM
	// Diffuse.R    | Diffuse.G     | Diffuse.B     | Ambient.B | RGBA8_UNORM
	// Specular.R   | Specular.G    | Specular.B    | Shininess | RGBA8_UNORM

	output.Ambient = ambient.rg;
	output.Diffuse = float4(albedo.rgb * Material.Diffuse, ambient.b);
	output.Specular = float4(specular, Material.Shininess);

	return output;
}
