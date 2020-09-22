//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const float3 SunDir = float3(-1, 1, 1);

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------
Texture2D txAmbient : register(t0);
Texture2D txAlbedo : register(t1);
Texture2D txSpecular : register(t2);

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

cbuffer TransformCB : register(b1)
{
	matrix World;
}

cbuffer MaterialConstants : register(b2)
{
	float3 Ambient;
	float3 Diffuse;
	float3 Specular;
	float Shininess;
	float Alpha;
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
	float3 WorldPos : TEXCOORD1;
};

struct PS_OUTPUT
{
	float4 Ambient: SV_Target0;
	float4 Diffuse: SV_Target1;
	float4 Specular: SV_Target2;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	matrix MVP = mul(mul(Proj, View), World);
	output.Pos = mul(MVP, float4(input.Pos, 1.f));
	output.Normal = input.Normal;
	output.Tex = input.Tex;
	output.WorldPos = mul(World, float4(input.Pos, 1)).xyz;
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

	float3 ambient = txAmbient.Sample(texSampler, input.Tex).rgb;
	float4 albedo = txAlbedo.Sample(texSampler, input.Tex).rgba;
	float3 specular = txSpecular.Sample(texSampler, input.Tex).rgb;
	float3 normal = normalize(input.Normal);

	// GBuffer Layout
	//     8        |       8       |       8       |      8
	// Ambient.R    | Ambient.G     | Ambient.B     | Normal.X  | RGBA8_UNORM
	// Diffuse.R    | Diffuse.G     | Diffuse.B     | Normal.Y  | RGBA8_UNORM
	// Specular.R   | Specular.G    | Specular.B    | Shininess | RGBA8_UNORM

	float2 n = float32x3_to_oct(normal.xyz) * 0.5 + 0.5;
	output.Ambient = float4(ambient * Ambient, n.x);
	output.Diffuse = float4(albedo.rgb * Diffuse, n.y);
	output.Specular = float4(specular * Specular, Shininess);

	return output;
}
