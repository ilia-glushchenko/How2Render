#include "CBuffers.fx"
#include "Helpers.fx"

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
Texture2D depthBufferTexture: register(t0);
Texture2D normalBufferTexture: register(t1);
Texture2D ambientGbufferTexture: register(t2);
Texture2D diffuseGbufferTexture: register(t3);
Texture2D specularGbufferTexture: register(t4);
Texture2D aoTexture: register(t5);
Texture2D shadowDepthTexture : register(t6);
Texture2D noiseTexture : register(t7);

SamplerState pointSampler : register(s0);
SamplerComparisonState depthSampler : register(s1);

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
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = float4(input.Pos, 1.f);
	output.Normal = float3(input.Normal);
	output.Tex = input.Tex;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float PercentageCloserFiltering(float4 shadowPos, float radius, float bias, float theta)
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
	shadowDepthTexture.GetDimensions(width, height);
	float2 normalizedRadius = float2(radius, radius) / float2(width, height);
	shadowPos.z -= bias;
	float2 shadowDepthTexCoords = float2(0.5 + shadowPos.x * 0.5, 0.5 - shadowPos.y * 0.5);

	float sum = 0.;
	for (uint i = 0; i < Shadows.PcfKernelSize; ++i)
	{
		// Compare z coordinate with depth stored in the shadow map
		float2 offset = poisson[i] * normalizedRadius;
		offset = RotateVector2D(offset, sin(theta), cos(theta));
		sum += shadowDepthTexture.SampleCmpLevelZero(
			depthSampler, shadowDepthTexCoords + offset, shadowPos.z);
	}

	return sum / (float)Shadows.PcfKernelSize;
}

float4 PS(PS_INPUT input) : SV_Target
{
	float Depth = depthBufferTexture.Sample(pointSampler, input.Tex).r;
	float2 NormalXY = normalBufferTexture.Sample(pointSampler, input.Tex).xy;
	float2 AmbientRG = ambientGbufferTexture.Sample(pointSampler, input.Tex).rg;
	float4 DiffuseAmbientB = diffuseGbufferTexture.Sample(pointSampler, input.Tex);
	float4 SpecShininess = specularGbufferTexture.Sample(pointSampler, input.Tex);

	float ao = aoTexture.Sample(pointSampler, input.Tex).r;
	float3 Kambient = float3(AmbientRG.rg, DiffuseAmbientB.w);
	float3 Kdiff = DiffuseAmbientB.rgb;
	float3 Kspec = SpecShininess.rgb;
	float Shininess = SpecShininess.a;

	float3 p = WorldPosFromDepth(Depth, input.Tex, Camera.inverseProj, Camera.inverseView);
	float3 n = OctToFloat32x3(NormalXY * 2.f - 1);
	float3 v = normalize(Camera.PosWorld.xyz - p);
	float3 l = normalize(Lights.ViewDir.xyz);

	float shadow = 1.f;
	if (Debug.ShadowMappingEnabled)
	{
		// No need to divide by w because of orthographic projection
		float4 shadowPos = mul(Lights.ViewProj, float4(p, 1));
		if (Shadows.PcfEnabled)
		{
			uint2 noiseTextureDimensions;
			noiseTexture.GetDimensions(noiseTextureDimensions.x, noiseTextureDimensions.y);

			float theta = 6.28 * noiseTexture.SampleLevel(
				pointSampler, RenderTarget.Dimensions / noiseTextureDimensions * input.Tex, 0).x;
			shadow = PercentageCloserFiltering(shadowPos, Shadows.PcfRadius, Shadows.Bias, theta);
		}
		else
		{
			// Compare z coordinate with depth stored in the shadow map
			shadowPos.z -= Shadows.Bias;
			float2 shadowDepthTexCoords = float2(0.5 + shadowPos.x * 0.5, 0.5 - shadowPos.y * 0.5);
			shadow = shadowDepthTexture.SampleCmpLevelZero(depthSampler, shadowDepthTexCoords, shadowPos.z);
		}
	}

	// Phong BRDF
	float NdL = max(dot(n, l), 0.);
	float3 r = reflect(-l, n);
	float RdV = max(dot(r, v), 0.);
	float3 color = Kambient * ao + (Kdiff * NdL * ao + pow(RdV, Shininess) * Kspec) * shadow;

	return float4(color, 1);
}
