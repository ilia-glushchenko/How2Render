
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
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float3 albedo = txAlbedo.Sample(texSampler, input.Tex).rgb;
	return float4(albedo, Alpha);
}
