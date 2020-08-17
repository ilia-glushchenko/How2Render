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
Texture2D txBump : register(t3);
Texture2D txShadowMap : register(t4);

SamplerState texSampler : register(s0);
SamplerComparisonState depthSampler : register(s1);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix WorldView;
	matrix WorldViewProj;
	matrix NormalMatrix;
	matrix ShadowProj;
	float4 LightViewPos;
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
	float3 ViewNormal : NORMAL;
	float2 Tex : TEXCOORD0;
	float4 WorldPos : TEXCOORD1;
	float3 ViewPos : TEXCOORD2;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(WorldViewProj, input.Pos);
	output.WorldPos = mul(World, input.Pos);
	output.ViewPos = mul(WorldView, input.Pos).xyz;
	output.ViewNormal = mul((float3x3)NormalMatrix, input.Normal);
	output.Tex = input.Tex;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float3 ambient = txAmbient.Sample(texSampler, input.Tex).rgb;
	float4 albedo = txAlbedo.Sample(texSampler, input.Tex);
	float3 specular = txSpecular.Sample(texSampler, input.Tex).rgb;
	float depth = txShadowMap.Sample(texSampler, input.Tex).r;

	const float alphaThreshold = 0.1;
	if (albedo.a < alphaThreshold)
		discard;

	float3 n = normalize(input.ViewNormal);
	float3 l = normalize(LightViewPos - input.ViewPos.xyz);
	float3 v = normalize(-input.ViewPos.xyz);

	float3 Kambient = ambient * Ambient;
	float3 Kdiff = albedo.rgb * Diffuse;
	float3 Kspec = specular * Specular;

	// Compare .z coordinate with depth stored in shadow map
	float4 shadowPos = mul(ShadowProj, input.WorldPos);
	shadowPos.z -= 0.05; // Bias
	shadowPos /= shadowPos.w;
	float shadow = txShadowMap.SampleCmpLevelZero(depthSampler, shadowPos.xy, shadowPos.z);

	// Phong BRDF
	float NdL = max(dot(n, l), 0.);
	float3 r = reflect(-l, n);
	float RdV = max(dot(r, v), 0.);
	float3 color = Kambient + (Kdiff * NdL + pow(RdV, Shininess) * Kspec) * shadow;

	return float4(color, albedo.a);
}
