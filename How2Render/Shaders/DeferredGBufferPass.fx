//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const float3 SunDir = float3(-1, 1, 1);

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
Texture2D txAmbient : register(t0);
Texture2D txAlbedo : register(t1);
Texture2D txSpecular : register(t2);

SamplerState texSampler : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

cbuffer PerInstanceCB : register(b0)
{
	matrix World;
};

cbuffer PerMaterialCB : register(b1)
{
	float3 Ambient;
	float3 Diffuse;
	float3 Specular;
	float Shininess;
	float Alpha;
};

cbuffer PerFrameCB : register(b2)
{
	matrix View;
	matrix Proj;
	matrix inverseView;
	matrix inverseProj;
	float4 CameraPos;
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
	float2 Tex : TEXCOORD0;
	float3 WorldPos : TEXCOORD1;
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
	matrix MVP = mul(mul(Proj, View), World);
	output.Pos = mul(MVP, float4(input.Pos, 1.f));
	output.Tex = input.Tex;
	output.WorldPos = mul(World, float4(input.Pos, 1)).xyz;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

PS_OUTPUT PS(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;

	float3 ambient = txAmbient.Sample(texSampler, input.Tex).rgb * Ambient;
	float4 albedo = txAlbedo.Sample(texSampler, input.Tex).rgba;
	float3 specular = txSpecular.Sample(texSampler, input.Tex).rgb * Specular;

	// GBuffer Layout
	//     8        |       8       |       8       |      8
	// Ambient.R    | Ambient.G     |               |           | RG8_UNORM
	// Diffuse.R    | Diffuse.G     | Diffuse.B     | Ambient.B | RGBA8_UNORM
	// Specular.R   | Specular.G    | Specular.B    | Shininess | RGBA8_UNORM

	output.Ambient = ambient.rg;
	output.Diffuse = float4(albedo.rgb * Diffuse, ambient.b);
	output.Specular = float4(specular, Shininess);

	return output;
}
