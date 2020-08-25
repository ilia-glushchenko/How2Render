//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
#define USE_PCF
#define PCF_SAMPLES 16

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
float pcf(float4 shadowPos, float radius, float bias)
{
	const float2 poisson[PCF_SAMPLES] =
	{
		float2(-0.376812, 0.649265),
		float2(-0.076855, -0.632508),
		float2(-0.833781, -0.268513),
		float2(0.398413, 0.027787),
		float2(0.360999, 0.766915),
		float2(0.584715, -0.809986),
		float2(-0.238882, 0.067867),
		float2(0.824410, 0.543863),
		float2(0.883033, -0.143517),
		float2(-0.581550, -0.809760),
		float2(-0.682282, 0.223546),
		float2(0.438031, -0.405749),
		float2(0.045340, 0.428813),
		float2(-0.311559, -0.328006),
		float2(-0.054146, 0.935302),
		float2(0.723339, 0.196795)
	};
	uint width, height;
	txShadowMap.GetDimensions(width, height);
	float invSize = float2(1., 1.)/float2(width, height);
	float invNormRadius = radius * invSize;
	shadowPos.z -= bias;

	float sum = 0.;
	for (int i = 0; i < PCF_SAMPLES; ++i)
	{
		float2 offset = poisson[i] * invNormRadius;
		sum += txShadowMap.SampleCmpLevelZero(depthSampler, shadowPos.xy + offset, shadowPos.z);
	}

	return sum/(float)PCF_SAMPLES;
}

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
	float4 clipPos = mul(ShadowProj, input.WorldPos);
	const float bias = 0.00025;
#ifdef USE_PCF
	float shadow = pcf(clipPos/clipPos.w, 4.0, bias);
#else
	float3 shadowPos = clipPos.xyz/clipPos.w;
	shadowPos.z -= bias;
	float shadow = txShadowMap.SampleCmpLevelZero(depthSampler, shadowPos.xy, shadowPos.z);
#endif

	// Phong BRDF
	float NdL = max(dot(n, l), 0.);
	float3 r = reflect(-l, n);
	float RdV = max(dot(r, v), 0.);
	float3 color = Kambient + (Kdiff * NdL + pow(RdV, Shininess) * Kspec) * shadow;

	return float4(color, albedo.a);
}
