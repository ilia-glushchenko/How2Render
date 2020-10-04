//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
#define PCF_SAMPLES 16
#define AO_SAMPLES 64
static const float AlphaThreshold = 0.3f;
static const float PI = 3.141592f;

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer PerInstanceCB : register(b0)
{
	struct TransformCB
	{
		matrix World;
	} Transform;
};

cbuffer PerMaterialCB : register(b1)
{
	struct MaterialCB
	{
		float3 Ambient;
		float3 Diffuse;
		float3 Specular;
		float Shininess;
		float Alpha;
		uint NormalMapAvailabled;
	} Material;
};

cbuffer PerPassCB : register(b2)
{
	struct RenderTargetCB
	{
		uint2 Dimensions;
	} RenderTarget;
}

cbuffer PerFrameCB : register(b3)
{
	struct CameraCB
	{
		matrix View;
		matrix Proj;
		matrix inverseView;
		matrix inverseProj;
		float4 PosWorld;
	} Camera;
};

cbuffer InfrequentCB : register(b4)
{
	struct LightsCB
	{
		matrix ViewProj;
		float4 ViewDir;
	} Lights;

	struct SsaoCB
	{
		uint KernelSize;
		float KernelRadius;
		float Bias;
		float4 Kernel[64];
	} SSAO;

	struct ShadowsCB
	{
		uint PcfEnabled;
		uint PcfKernelSize;
		float PcfRadius;
		float Bias;
	} Shadows;

	struct DebugCB
	{
		uint NormalMappingEnabled;
		uint ShadowMappingEnabled;
		uint FinalOutputIndex;
	} Debug;
};
