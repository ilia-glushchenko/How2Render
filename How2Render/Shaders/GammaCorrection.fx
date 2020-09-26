//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
Texture2D linearColorTexture : register(t0);
SamplerState textureSampler : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer InfrequentCB : register(b3)
{
	uint FinalOutputIndex;
	uint SsaoKernelSize;
	float SsaoKernelRadius;
	float SsaoBias;
	float4 SsaoKernel[64];
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

	output.Pos = float4(input.Pos, 1.f);
	output.Tex = input.Tex;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float3 linearColor = linearColorTexture.Sample(textureSampler, input.Tex).rgb;
	float3 nonLinearColor = pow(linearColor, 1.0f / 2.2f);
	return float4(nonLinearColor, 1.);
}
