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
Texture2D txNormal : register(t3);

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
float4 PS(PS_INPUT input) : SV_Target
{
	float3 ambient = txAmbient.Sample(texSampler, input.Tex).rgb;
	float3 albedo = txAlbedo.Sample(texSampler, input.Tex).rgb;
	float3 specular = txSpecular.Sample(texSampler, input.Tex).rgb;
	float3 micronormal = txNormal.Sample(texSampler, input.Tex).rgb;

	// Compute per-pixel cotangent frame
	float3 normal = normalize(input.Normal);
	float3x3 TBN = cotangentFrame(normal, input.WorldPos, input.Tex);

	float3 v = normalize(CameraPos.xyz - input.WorldPos);
	float3 l = normalize(SunDir);

	// In terms of normal maps, the difference result in how the green channel of a RGB texture should be interpreted.
	// OpenGL expects the first pixel to be at the bottom while DirectX expects it to be at the top
	// https://docs.substance3d.com/bake/what-is-the-difference-between-the-opengl-and-directx-normal-format-182256965.html
	micronormal.y = 1. - micronormal.y;
	// Transform normal from texture space to object space
	float3 n = mul(micronormal * 2. - 1., TBN);

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
