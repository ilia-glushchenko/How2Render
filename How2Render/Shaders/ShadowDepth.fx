#include "CBuffers.fx"

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
Texture2D txAmbient : register(t0);
SamplerState texSampler : register(s0);

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

	matrix WVP = mul(Lights.ViewProj, Transform.World);
	output.Pos = mul(WVP, float4(input.Pos, 1.f));
	output.Tex = input.Tex;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
void PS(PS_INPUT input)
{
	const float threshold = 0.3f;

	if (txAmbient.Sample(texSampler, input.Tex).a < threshold)
	{
		discard;
	}
}
