//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register(t0);
SamplerState texSampler : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix WorldViewProj;
    uint ScreenWidth;
    uint ScreenHeight;
    uint FrameCount;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
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
    float4 pos = input.Pos;
    output.Pos = mul(WorldViewProj, pos);
    output.Normal = input.Normal;
    output.Tex.x = input.Tex.x;
    output.Tex.y = 1. - input.Tex.y;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
    float3 color = txDiffuse.Sample(texSampler, input.Tex).rgb;
    return float4(color, 1.0);
}
