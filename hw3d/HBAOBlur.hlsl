#include "Constants.hlsli"
#include "DeferredCommon.hlsli"

#define USE_MAD_OPT 1
#define USE_DEPTH_SLOPE 1
#define ENABLE_SHARPNESS_PROFILE 1
#define KERNEL_RADIUS 4
#define USE_ADAPTIVE_SAMPLING 1

Texture2D coarseAO : register(t0);
Texture2D coarseAOX : register(t1);
Texture2D scenedepth : register(t8);
SamplerState splr;

cbuffer CBufProperties : register(b10)
{
	float ViewDepthThresholdNegInv;
	float ViewDepthThresholdSharpness;
	float NegInvR2;
	float RadiusToScreen;
	float BackgroundAORadiusPixels;
	float ForegroundAORadiusPixels;
	float NDotVBias;
	float2 AORes;
	float2 InvAORes;
	float SmallScaleAOAmount;
	float LargeScaleAOAmount;
	float PowerExponent;
	float BlurViewDepth0;
	float BlurViewDepth1;
	float BlurSharpness0;
	float BlurSharpness1;
};

cbuffer CBufProperties2 : register(b11)
{
	bool isHorizontal;
};

struct CenterPixelData
{
	float2 UV;
	float Depth;
	float Sharpness;
#if USE_MAD_OPT
	float Scale;
	float Bias;
#endif
};

float GetSharpness(float ViewDepth)
{
#if ENABLE_SHARPNESS_PROFILE
	float lerpFactor = (ViewDepth - BlurViewDepth0) / (BlurViewDepth1 - BlurViewDepth0);
	return lerp(BlurSharpness0, BlurSharpness1, saturate(lerpFactor));
#else
	return BlurSharpness1;
#endif
}

float CrossBilateralWeight(float R, float SampleDepth, float DepthSlope, CenterPixelData Center)
{
	const float BlurSigma = ((float)KERNEL_RADIUS + 1.0) * 0.5;
	const float BlurFalloff = 1.0 / (2.0 * BlurSigma * BlurSigma);

#if USE_DEPTH_SLOPE
	SampleDepth -= DepthSlope * R;
#endif

#if USE_MAD_OPT
	float DeltaZ = SampleDepth * Center.Scale + Center.Bias;
#else
	float DeltaZ = (SampleDepth - Center.Depth) * Center.Sharpness;
#endif

	return exp2(-R * R * BlurFalloff - DeltaZ * DeltaZ);
}

void ProcessSample(float2 AOZ,
	float R,
	float DepthSlope,
	CenterPixelData Center,
	inout float TotalAO,
	inout float TotalW)
{
	float AO = AOZ.x;
	float Z = AOZ.y;

	float W = CrossBilateralWeight(R, Z, DepthSlope, Center);
	TotalAO += W * AO;
	TotalW += W;
}

void ProcessRadius(float R0,
	float2 DeltaUV,
	float DepthSlope,
	CenterPixelData Center,
	inout float TotalAO,
	inout float TotalW)
{
#if USE_ADAPTIVE_SAMPLING
	float R = R0;

	[unroll]
	for (; R <= KERNEL_RADIUS / 2; R += 1)
	{
		float2 UV = R * DeltaUV + Center.UV;

		float _AOZ = 1;
		if (isHorizontal)
			_AOZ = coarseAO.SampleLevel(splr, UV, 0).x;
		else
			_AOZ = coarseAOX.SampleLevel(splr, UV, 0).x;
		float CenterDepth = ConvertToLinearDepth(scenedepth.SampleLevel(splr, UV, 0).x);

		float2 AOZ = float2(_AOZ, CenterDepth);
		ProcessSample(AOZ, R, DepthSlope, Center, TotalAO, TotalW);
	}

	[unroll]
	for (; R <= KERNEL_RADIUS; R += 2)
	{
		float2 UV = (R + 0.5) * DeltaUV + Center.UV;

		float _AOZ = 1;
		if (isHorizontal)
			_AOZ = coarseAO.SampleLevel(splr, UV, 0).x;
		else
			_AOZ = coarseAOX.SampleLevel(splr, UV, 0).x;
		float CenterDepth = ConvertToLinearDepth(scenedepth.SampleLevel(splr, UV, 0).x);

		float2 AOZ = float2(_AOZ, CenterDepth);
		ProcessSample(AOZ, R, DepthSlope, Center, TotalAO, TotalW);
	}
#else
	[unroll]
	for (float R = R0; R <= KERNEL_RADIUS; R += 1)
	{
		float2 UV = R * DeltaUV + Center.UV;

		float _AOZ = 1;
		if (isHorizontal)
			_AOZ = coarseAO.SampleLevel(splr, UV, 0).x;
		else
			_AOZ = coarseAOX.SampleLevel(splr, UV, 0).x;
		float CenterDepth = ConvertToLinearDepth(scenedepth.SampleLevel(splr, UV, 0).x);

		float2 AOZ = float2(_AOZ, CenterDepth);
		ProcessSample(AOZ, R, DepthSlope, Center, TotalAO, TotalW);
	}
#endif
}

#if USE_DEPTH_SLOPE
void ProcessRadius1(float2 DeltaUV,
	CenterPixelData Center,
	inout float TotalAO,
	inout float TotalW)
{
	float AOZ = 1;
	if (isHorizontal)
		AOZ = coarseAO.SampleLevel(splr, Center.UV + DeltaUV, 0).x;
	else
		AOZ = coarseAOX.SampleLevel(splr, Center.UV + DeltaUV, 0).x;
	float CenterDepth = ConvertToLinearDepth(scenedepth.SampleLevel(splr, Center.UV + DeltaUV, 0).x);

	float2 AODepth = float2(AOZ, CenterDepth);
	float DepthSlope = AODepth.y - Center.Depth;

	ProcessSample(AODepth, 1, DepthSlope, Center, TotalAO, TotalW);
	ProcessRadius(2, DeltaUV, DepthSlope, Center, TotalAO, TotalW);
}
#endif

float ComputeBlur(float2 stc,
	float2 DeltaUV)
{
	float AOZ = 1;
	if (isHorizontal)
		AOZ = coarseAO.SampleLevel(splr, stc, 0).x;
	else
		AOZ = coarseAOX.SampleLevel(splr, stc, 0).x;
	float CenterDepth = ConvertToLinearDepth(scenedepth.SampleLevel(splr, stc, 0).x);

	CenterPixelData Center;
	Center.UV = stc;
	Center.Depth = CenterDepth;
	Center.Sharpness = GetSharpness(CenterDepth);

#if USE_MAD_OPT
	Center.Scale = Center.Sharpness;
	Center.Bias = -Center.Depth * Center.Sharpness;
#endif

	float TotalAO = AOZ;
	float TotalW = 1.0;

#if USE_DEPTH_SLOPE
	ProcessRadius1(DeltaUV, Center, TotalAO, TotalW);
	ProcessRadius1(-DeltaUV, Center, TotalAO, TotalW);
#else
	float DepthSlope = 0;
	ProcessRadius(1, DeltaUV, DepthSlope, Center, TotalAO, TotalW);
	ProcessRadius(1, -DeltaUV, -DepthSlope, Center, TotalAO, TotalW);
#endif

	return TotalAO / TotalW;
}

float4 main(float2 uv : Texcoord) : SV_Target
{	
	float AO = 1;
	if(isHorizontal)
    	AO = ComputeBlur(uv, float2(screenInfo.z, 0));
	else
		AO = ComputeBlur(uv, float2(0, screenInfo.w));
	if (isHorizontal)
		return AO;
	else
	{
		AO = pow(saturate(AO), PowerExponent);
		return float4(0, 0, 0, 1 - AO);
	}
}