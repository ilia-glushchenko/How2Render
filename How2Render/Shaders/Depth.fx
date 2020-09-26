//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const float AlphaThreshold = 0.3f;

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
#ifdef ENABLE_TRANSPARENCY
Texture2D ambientTexture : register(t0);
SamplerState textureSampler : register(s0);
#endif

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer PerInstanceCB : register(b0)
{
	matrix World;
};

cbuffer PerFrameCB : register(b2)
{
	matrix View;
	matrix Proj;
	matrix inverseView;
	matrix inverseProj;
	float4 CameraPos;
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
	float3 Normal : NORMAL;
	float2 Tex: TEXCOORD0;
};

struct PS_OUTPUT
{
	float2 Normal: SV_Target0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	matrix WVP = mul(mul(Proj, View), World);
	output.Pos = mul(WVP, float4(input.Pos, 1.f));
	output.Normal = mul(World, float4(input.Normal, 0));
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

// Assume normalized input. Output is on [-1, 1] for each component.
float2 float32x3_to_oct(float3 v)
{
	// Project the sphere onto the octahedron, and then onto the xy plane
	float2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
	// Reflect the folds of the lower hemisphere over the diagonals
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * signNotZero(p)) : p;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

PS_OUTPUT PS(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;

#ifdef ENABLE_TRANSPARENCY
	if (ambientTexture.Sample(textureSampler, input.Tex).a < AlphaThreshold) {
		discard;
	}
#endif

	output.Normal = float32x3_to_oct(normalize(input.Normal)) * 0.5f + 0.5;

	return output;
}
