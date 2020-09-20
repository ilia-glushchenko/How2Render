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
cbuffer CameraCB : register(b0)
{
	matrix View;
	matrix Proj;
	float4 CameraPos;
}

cbuffer TransformCB : register(b1)
{
	matrix World;
}

cbuffer MaterialConstants : register(b2)
{
	float3 Ambient;
	float3 Diffuse;
	float3 Specular;
	float Shininess;
	float Alpha;
}

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
	float4 Position: SV_Target0;
	float4 Normal: SV_Target1;
	float4 Ambient: SV_Target2;
	float4 Diffuse: SV_Target3;
	float4 Specular: SV_Target4;
	float4 Shininess: SV_Target5;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	matrix MVP = mul(mul(Proj, View), World);
	output.Pos = mul(MVP, float4(input.Pos, 1.f));
	output.Normal = input.Normal;
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

	float3 ambient = txAmbient.Sample(texSampler, input.Tex).rgb;
	float4 albedo = txAlbedo.Sample(texSampler, input.Tex).rgba;
	float3 specular = txSpecular.Sample(texSampler, input.Tex).rgb;
	float3 normal = normalize(input.Normal);
	normal = (normal + 1.f) * 0.5f;

	output.Position = float4(input.Pos.xyz, albedo.a);
	output.Normal = float4(normal, albedo.a);
	output.Ambient = float4(ambient * Ambient, albedo.a);
	output.Diffuse = float4(albedo * Diffuse, albedo.a);
	output.Specular = float4(specular * Specular, albedo.a);
	output.Shininess = float4(Shininess, Shininess, Shininess, albedo.a);

	return output;
}
