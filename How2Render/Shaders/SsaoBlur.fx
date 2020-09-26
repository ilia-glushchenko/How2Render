//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D<float> sourceAO : register(t0);
RWTexture2D<float> blurredAO : register(u0);
SamplerState linearSampler : register(s0);

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

//--------------------------------------------------------------------------------------
// Compute Shader
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Efficient Gaussian blur with linear sampling
// http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
//--------------------------------------------------------------------------------------
float OneDirectionGaussianBlur(uint3 dispatchThreadId, float2 dir)
{
    const float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
    const float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

    uint2 aoTextureDimensions;
    sourceAO.GetDimensions(aoTextureDimensions.x, aoTextureDimensions.y);
    float2 pointScreenSpace = float2(dispatchThreadId.x, dispatchThreadId.y);
    float2 pointPixelSpace = pointScreenSpace / aoTextureDimensions;

    float ao = sourceAO.SampleLevel(linearSampler, pointPixelSpace, 0).r * weight[0];
    for (int i = 1; i < 3; i++)
    {
        ao +=
            sourceAO.SampleLevel(linearSampler, (pointScreenSpace + float2(offset[i], offset[i]) * dir) / aoTextureDimensions, 0)
            * weight[i];
        ao +=
            sourceAO.SampleLevel(linearSampler, (pointScreenSpace - float2(offset[i], offset[i]) * dir) / aoTextureDimensions, 0)
            * weight[i];
    }

    return ao;
}

[numthreads(32, 32, 1)]
void GaussianBlurVertical(
    uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint groupIndex : SV_GroupIndex)
{
    blurredAO[dispatchThreadId.xy] = OneDirectionGaussianBlur(dispatchThreadId, float2(1, 0));
}

[numthreads(32, 32, 1)]
void GaussianBlurHorizontal(
    uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint groupIndex : SV_GroupIndex)
{
    blurredAO[dispatchThreadId.xy] = OneDirectionGaussianBlur(dispatchThreadId, float2(0, 1));
}
