//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const float PI = 3.141592f;

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D<float> depthTexture : register(t0);
Texture2D<float2> normalTexture : register(t1);
Texture2D<float> noiseTexture : register(t2);
RWTexture2D<float> aoTexture : register(u0);
SamplerState wrapPointSampler : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer PerFrameCB : register(b2)
{
    matrix View;
    matrix Proj;
    matrix inverseView;
    matrix inverseProj;
    float4 CameraPos;
};

cbuffer InfrequentCB : register(b3)
{
    uint FinalOutputIndex;
    uint SsaoKernelSize;
    float SsaoKernelRadius;
    float SsaoBias;
    float4 SsaoKernel[64];
};

//--------------------------------------------------------------------------------------
// Compute Shader
//--------------------------------------------------------------------------------------

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

float3 ViewPosFromDepth(float depth, float2 TexCoord)
{
    float4 clipSpacePosition = float4((TexCoord * 2.0 - 1.0) * float2(1, -1), depth, 1.0);
    float4 viewSpacePosition = mul(inverseProj, clipSpacePosition);
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}

[numthreads(32, 32, 1)]
void SSAO(
    uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint groupIndex : SV_GroupIndex)
{
    uint2 aoTextureDimensions;
    aoTexture.GetDimensions(aoTextureDimensions.x, aoTextureDimensions.y);
    uint2 noiseTextureDimensions;
    noiseTexture.GetDimensions(noiseTextureDimensions.x, noiseTextureDimensions.y);

    float2 pointScreenSpace = float2(dispatchThreadId.x, dispatchThreadId.y);
    float2 pointPixelSpace = pointScreenSpace / aoTextureDimensions;

    float pointDepth = depthTexture[pointScreenSpace];
    float3 pointPosView = ViewPosFromDepth(pointDepth, pointPixelSpace);

    float3 normalWorld = oct_to_float32x3(normalTexture[pointScreenSpace] * 2.f - 1);
    float3 normalView = normalize(mul(View, float4(normalWorld, 0)).xyz);

    float alpha = 6.28 * noiseTexture.SampleLevel(wrapPointSampler, pointScreenSpace / noiseTextureDimensions, 0).x;
    float3 random = float3(-sin(alpha), cos(alpha), 0);
    float3 tangent = normalize(random - normalView * dot(random, normalView));
    float3 bitangent = cross(normalView, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normalView);

    float occludingPoints = 0;
    for (uint i = 0; i < SsaoKernelSize; i++)
    {
        float3 offset = mul(SsaoKernel[i].xyz, TBN);
        float4 samplePoint = float4(pointPosView + offset * SsaoKernelRadius, 1);
        samplePoint = mul(Proj, samplePoint);
        samplePoint.xyz /= samplePoint.w;

        float2 samplePointScreenSpace = float2(
            0.5f + (samplePoint.x * 0.5f),
            0.5f - (samplePoint.y * 0.5f));
        float sampleDepth = depthTexture[round(samplePointScreenSpace * aoTextureDimensions)];
        float3 samplePointVec = ViewPosFromDepth(sampleDepth, samplePointScreenSpace) - pointPosView;

        float rangeCheck = smoothstep(0.0, 1.0, SsaoKernelRadius / length(samplePointVec));
        occludingPoints += float(sampleDepth < samplePoint.z - SsaoBias) * rangeCheck;
    }
    float occlusionFactor = 1 - occludingPoints / float(SsaoKernelSize);

    aoTexture[dispatchThreadId.xy] = occlusionFactor;
}
