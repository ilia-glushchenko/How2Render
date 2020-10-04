#include "CBuffers.fx"
#include "Helpers.fx"

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D<float> depthTexture : register(t0);
Texture2D<float2> normalTexture : register(t1);
Texture2D<float> noiseTexture : register(t2);
RWTexture2D<float> aoTexture : register(u0);
SamplerState wrapPointSampler : register(s0);

//--------------------------------------------------------------------------------------
// Compute Shader
//--------------------------------------------------------------------------------------

[numthreads(32, 32, 1)]
void ComputeSSAO(
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
    float3 pointPosView = ViewPosFromDepth(pointDepth, pointPixelSpace, Camera.inverseProj);

    float3 normalWorld = OctToFloat32x3(normalTexture[pointScreenSpace] * 2.f - 1);
    float3 normalView = normalize(mul(Camera.View, float4(normalWorld, 0)).xyz);

    float alpha = 6.28 * noiseTexture.SampleLevel(wrapPointSampler, pointScreenSpace / noiseTextureDimensions, 0).x;
    float3 random = float3(-sin(alpha), cos(alpha), 0);
    float3 tangent = normalize(random - normalView * dot(random, normalView));
    float3 bitangent = cross(normalView, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normalView);

    float occludingPoints = 0;
    for (uint i = 0; i < SSAO.KernelSize; i++)
    {
        float3 offset = mul(SSAO.Kernel[i].xyz, TBN);
        float4 samplePoint = float4(pointPosView + offset * SSAO.KernelRadius, 1);
        samplePoint = mul(Camera.Proj, samplePoint);
        samplePoint.xyz /= samplePoint.w;

        float2 samplePointScreenSpace = float2(
            0.5f + (samplePoint.x * 0.5f),
            0.5f - (samplePoint.y * 0.5f));
        float sampleDepth = depthTexture[round(samplePointScreenSpace * aoTextureDimensions)];
        float3 samplePointVec = ViewPosFromDepth(sampleDepth, samplePointScreenSpace, Camera.inverseProj) - pointPosView;

        float rangeCheck = smoothstep(0.0, 1.0, SSAO.KernelRadius / length(samplePointVec));
        occludingPoints += float(sampleDepth < samplePoint.z - SSAO.Bias) * rangeCheck;
    }
    float occlusionFactor = 1 - occludingPoints / float(SSAO.KernelSize);

    aoTexture[dispatchThreadId.xy] = occlusionFactor;
}
