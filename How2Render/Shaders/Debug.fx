//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
Texture2D gammaCorrectedTexture : register(t0);
Texture2D depthTexture : register(t1);
Texture2D aoPassTexture : register(t2);
Texture2D gBuffersNormalXY : register(t3);
Texture2D gBuffersAmbientRG : register(t4);
Texture2D gBuffersDiffuseAmbientB : register(t5);
Texture2D gBuffersSpecularShininess : register(t6);

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
	float3 color = float3(0, 0, 0);

	if (FinalOutputIndex == 0)
	{
		color = gammaCorrectedTexture.Sample(textureSampler, input.Tex).rgb;
	}
	else if (FinalOutputIndex == 1)
	{
		color = pow(abs(depthTexture.Sample(textureSampler, input.Tex).rrr), 100);
	}
	else if (FinalOutputIndex == 2)
	{
		color = float3(gBuffersNormalXY.Sample(textureSampler, input.Tex).xy, 0);
	}
	else if (FinalOutputIndex == 3)
	{
		color = aoPassTexture.Sample(textureSampler, input.Tex).rrr;
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

		if (FinalOutputIndex == 4)
		{
			color = Kambient;
		}
		else if (FinalOutputIndex == 5)
		{
			color = Kdiff;
		}
		else if (FinalOutputIndex == 6)
		{
			color = Kspec;
		}
		else if (FinalOutputIndex == 7)
		{
			color = float3(Shininess, Shininess, Shininess);
		}
	}

	return float4(color, 1.);
}
