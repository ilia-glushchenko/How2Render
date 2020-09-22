//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const float3 SunDir = float3(-1, 1, 1);

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
Texture2D DepthBufferTexture: register(t0);
Texture2D AmbientGbufferTexture: register(t1);
Texture2D DiffuseGbufferTexture: register(t2);
Texture2D SpecularGbufferTexture: register(t3);

SamplerState texSampler : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer CameraCB : register(b0)
{
	matrix View;
	matrix Proj;
	matrix inverseView;
	matrix inverseProj;
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
// Survey of Efficient Representations for Independent Unit Vectors
// http://jcgt.org/published/0003/02/01/
//--------------------------------------------------------------------------------------

// Returns +/- 1
float2 signNotZero(float2 v)
{
	return float2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

float3 oct_to_float32x3(float2 e)
{
	float3 v = float3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
	return normalize(v);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float3 WorldPosFromDepth(float depth, float2 TexCoord)
{
	float4 clipSpacePosition = float4((TexCoord * 2.0 - 1.0) * float2(1, -1), depth, 1.0);
	float4 viewSpacePosition = mul(inverseProj, clipSpacePosition);
	viewSpacePosition /= viewSpacePosition.w;
	float4 worldSpacePosition = mul(inverseView, viewSpacePosition);

	return worldSpacePosition.xyz;
}

float4 PS(PS_INPUT input) : SV_Target
{
	float depth = DepthBufferTexture.Sample(texSampler, input.Tex).r;
	float4 AmbientNX = AmbientGbufferTexture.Sample(texSampler, input.Tex);
	float4 DiffuseNY = DiffuseGbufferTexture.Sample(texSampler, input.Tex);
	float4 SpecShininess = SpecularGbufferTexture.Sample(texSampler, input.Tex);

	float3 Kdiff = DiffuseNY.rgb;
	float3 Kambient = AmbientNX.rgb;
	float3 Kspec = SpecShininess.rgb;
	float Shininess = SpecShininess.a;

	float3 n = normalize(oct_to_float32x3(float2(AmbientNX.w, DiffuseNY.w) * 2.f - 1));
	float3 v = normalize(CameraPos.xyz - WorldPosFromDepth(depth, input.Tex));
	float3 l = normalize(SunDir);

	// Phong BRDF
	float NdL = max(dot(n, l), 0.);
	float3 r = reflect(-l, n);
	float RdV = max(dot(r, v), 0.);
	float3 color = Kambient + Kdiff * NdL + pow(RdV, Shininess) * Kspec;

	return float4(color, 1);
}
