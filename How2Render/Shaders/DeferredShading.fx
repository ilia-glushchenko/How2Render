//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const float3 SunDir = float3(-1, 1, 1);

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
Texture2D PositionGbufferTexture: register(t0);
Texture2D NormalGbufferTexture: register(t1);
Texture2D AmbientGbufferTexture: register(t2);
Texture2D DiffuseGbufferTexture: register(t3);
Texture2D SpecularGbufferTexture: register(t4);
Texture2D ShininessGbufferTexture : register(t5);

SamplerState texSampler : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer CameraCB : register(b0)
{
	matrix View;
	matrix Proj;
	float4 CameraPos;
}

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
	float4 position = PositionGbufferTexture.Sample(texSampler, input.Tex);
	float4 normal = NormalGbufferTexture.Sample(texSampler, input.Tex);
	float3 Kambient = AmbientGbufferTexture.Sample(texSampler, input.Tex).rgb;
	float3 Kdiff = DiffuseGbufferTexture.Sample(texSampler, input.Tex).rgb;
	float3 Kspec = SpecularGbufferTexture.Sample(texSampler, input.Tex).rgb;
	float Shininess = ShininessGbufferTexture.Sample(texSampler, input.Tex).r;

	float3 n = (normal.xyz * 2.f) - 1.f;
	float3 v = normalize(CameraPos.xyz - position.xyz);
	float3 l = normalize(SunDir);

	// Phong BRDF
	float NdL = max(dot(n, l), 0.);
	float3 r = reflect(-l, n);
	float RdV = max(dot(r, v), 0.);
	float3 color = Kambient + Kdiff * NdL + pow(RdV, Shininess) * Kspec;

	return float4(color, 1.0);
}
