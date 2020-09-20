//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
Texture2D txLinearColorTexture : register(t0);
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
float4 PS(PS_INPUT input) : SV_Target
{
	float3 linearColor = txLinearColorTexture.Sample(texSampler, input.Tex).rgb;
	float3 nonLinearColor = pow(linearColor, 1.0f / 2.2f);

	return float4(nonLinearColor, 1.);
}
