#include "CBuffers.fx"
#include "Helpers.fx"

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const uint FinalImage = 0;
static const uint Depth = 1;
static const uint Normals = 2;
static const uint AO = 3;
static const uint ShadowMap = 4;
static const uint gBufferAmbient = 5;
static const uint gBufferDiffuse = 6;
static const uint gBufferSpecular = 7;
static const uint gBufferShininess = 8;

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
Texture2D gammaCorrectedTexture : register(t0);
Texture2D depthTexture : register(t1);
Texture2D aoPassTexture : register(t2);
Texture2D shadowMapTexture : register(t3);
Texture2D gBuffersNormalXY : register(t4);
Texture2D gBuffersAmbientRG : register(t5);
Texture2D gBuffersDiffuseAmbientB : register(t6);
Texture2D gBuffersSpecularShininess : register(t7);

SamplerState textureSampler : register(s0);

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

	output.Pos = float4(input.Pos, 1.f);
	output.Tex = input.Tex;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float3 color = float3(0, 0, 0);

	if (Debug.FinalOutputIndex == FinalImage)
	{
		color = gammaCorrectedTexture.Sample(textureSampler, input.Tex).rgb;
	}
	else if (Debug.FinalOutputIndex == Depth)
	{
		color = pow(abs(depthTexture.Sample(textureSampler, input.Tex).rrr), 100);
	}
	else if (Debug.FinalOutputIndex == Normals)
	{
		color = float3(gBuffersNormalXY.Sample(textureSampler, input.Tex).xy, 0);
	}
	else if (Debug.FinalOutputIndex == AO)
	{
		color = aoPassTexture.Sample(textureSampler, input.Tex).rrr;
	}
	else if (Debug.FinalOutputIndex == ShadowMap)
	{
		float depth = depthTexture.Sample(textureSampler, input.Tex).r;
		float3 p = WorldPosFromDepth(depth, input.Tex, Camera.inverseProj, Camera.inverseView);
		float4 shadowPos = mul(Lights.ViewProj, float4(p, 1));
		color = float3(shadowPos.zzz / shadowPos.w);
	}
	else
	{
		float2 AmbientRG = gBuffersAmbientRG.Sample(textureSampler, input.Tex).rg;
		float4 DiffuseAmbientB = gBuffersDiffuseAmbientB.Sample(textureSampler, input.Tex);
		float4 SpecShininess = gBuffersSpecularShininess.Sample(textureSampler, input.Tex);

		float3 Kambient = float3(AmbientRG.rg, DiffuseAmbientB.w);
		float3 Kdiff = DiffuseAmbientB.rgb;
		float3 Kspec = SpecShininess.rgb;
		float Shininess = SpecShininess.a;

		if (Debug.FinalOutputIndex == gBufferAmbient)
		{
			color = Kambient;
		}
		else if (Debug.FinalOutputIndex == gBufferDiffuse)
		{
			color = Kdiff;
		}
		else if (Debug.FinalOutputIndex == gBufferSpecular)
		{
			color = Kspec;
		}
		else if (Debug.FinalOutputIndex == gBufferShininess)
		{
			color = float3(Shininess, Shininess, Shininess);
		}
	}

	return float4(color, 1.);
}
