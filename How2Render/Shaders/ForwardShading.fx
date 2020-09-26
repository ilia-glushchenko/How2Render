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
Texture2D txAO : register(t3);

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
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 WorldPos : TEXCOORD1;
	float4 Pos2: TEXCOORD2;
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
	output.Pos2 = output.Pos;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float3 ambient = txAmbient.Sample(texSampler, input.Tex).rgb;
	float4 albedo = txAlbedo.Sample(texSampler, input.Tex).rgba;
	float3 specular = txSpecular.Sample(texSampler, input.Tex).rgb;

	float2 texCoordSS = float2(
		0.5 + (input.Pos2.x / input.Pos2.w * 0.5),
		0.5 - (input.Pos2.y / input.Pos2.w * 0.5)
	);
	float ao = txAO.Sample(texSampler, texCoordSS).r;

	float3 n = normalize(input.Normal);
	float3 v = normalize(CameraPos.xyz - input.WorldPos);
	float3 l = normalize(SunDir);

	float3 Kambient = ambient * Ambient;
	float3 Kdiff = albedo.rgb * Diffuse;
	float3 Kspec = specular * Specular;

	// Phong BRDF
	float NdL = max(dot(n, l), 0.);
	float3 r = reflect(-l, n);
	float RdV = max(dot(r, v), 0.);
	float3 color = (Kambient + Kdiff * NdL) * ao + pow(RdV, Shininess) * Kspec;

	return float4(color, albedo.a);
}
