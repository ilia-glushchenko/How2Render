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
cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix WorldViewProj;
	float4 CameraPos;
}

cbuffer MaterialConstants : register(b1)
{
	float3 Ambient;
	float3 Diffuse;
	float3 Specular;
	float Shininess;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 Pos : POSITION;
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

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(WorldViewProj, input.Pos);
	output.Normal = input.Normal;
	output.Tex = input.Tex;
	output.WorldPos = mul(World, input.Pos).xyz;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float3 ambient = txAmbient.Sample(texSampler, input.Tex).rgb;
	float3 albedo = txAlbedo.Sample(texSampler, input.Tex).rgb;
	float3 specular = txSpecular.Sample(texSampler, input.Tex).rgb;

	float3 n = normalize(input.Normal);
	float3 v = normalize(CameraPos.xyz - input.WorldPos);
	float3 l = normalize(SunDir);

	float3 Kambient = ambient * Ambient;
	float3 Kdiff = albedo * Diffuse;
	float3 Kspec = specular * Specular;

	// Phong BRDF
	float NdL = max(dot(n, l), 0.);
	float3 r = reflect(-l, n);
	float RdV = max(dot(r, v), 0.);
	float3 color = Kambient + Kdiff * NdL + pow(RdV, Shininess) * Kspec;

	return float4(color, 1.);
}
