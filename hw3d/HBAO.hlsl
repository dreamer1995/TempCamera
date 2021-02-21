#include "Constants.hlsli"
#include "DeferredCommon.hlsli"

#define MaxRadiusPixels 100.0
#define NUM_DIRECTIONS 8
#define NUM_STEPS 4
#define GFSDK_PI 3.14159265f

#define USE_MAD_OPT 1
#define USE_DEPTH_SLOPE 1
#define ENABLE_SHARPNESS_PROFILE 1
#define KERNEL_RADIUS 4

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
};

float3 UVToViewSpace(float2 uv, float z)
{
	uv = UVToViewA * uv + UVToViewB;
	return float3(uv * z, z);
}

float3 GetViewPos(float2 uv)
{
    //float z = ViewSpaceZFromDepth(DecodeDepth(tDepthBuffer.SampleLevel(sDepthBufferSampler, uv, 0)));
	float z = scenedepth.SampleLevel(splr, uv, 0);
	return UVToViewSpace(uv, z);
}

float3 MinDiff(float3 P, float3 Pr, float3 Pl)
{
	float3 V1 = Pr - P;
	float3 V2 = P - Pl;
	return (dot(V1, V1) < dot(V2, V2)) ? V1 : V2;
}

struct AORadiusParams
{
	float fRadiusPixels;
	float fNegInvR2;
};

void ScaleAORadius(inout AORadiusParams Params, float ScaleFactor)
{
	Params.fRadiusPixels *= ScaleFactor;
	Params.fNegInvR2 *= 1.0 / (ScaleFactor * ScaleFactor);
}

AORadiusParams GetAORadiusParams(float ViewDepth)
{
	AORadiusParams Params;
	Params.fRadiusPixels = RadiusToScreen / ViewDepth;
	Params.fNegInvR2 = NegInvR2;

    [branch]
	if (BackgroundAORadiusPixels != -1.f)
	{
		ScaleAORadius(Params, max(1.0, BackgroundAORadiusPixels / Params.fRadiusPixels));
	}

    [branch]
	if (ForegroundAORadiusPixels != -1.f)
	{
		ScaleAORadius(Params, min(1.0, ForegroundAORadiusPixels / Params.fRadiusPixels));
	}

	return Params;
}

float2 RotateDirection(float2 V, float2 RotationCosSin)
{
    // RotationCosSin is (cos(alpha),sin(alpha)) where alpha is the rotation angle
    // A 2D rotation matrix is applied (see https://en.wikipedia.org/wiki/Rotation_matrix)
	return float2(V.x * RotationCosSin.x - V.y * RotationCosSin.y,
                  V.x * RotationCosSin.y + V.y * RotationCosSin.x);
}

float Falloff(float DistanceSquare, AORadiusParams Params)
{
    // 1 scalar mad instruction
	return DistanceSquare * Params.fNegInvR2 + 1.0;
}

float ComputeAO(float3 P, float3 N, float3 S, AORadiusParams Params)
{
	float3 V = S - P;
	float VdotV = dot(V, V);
	float NdotV = dot(N, V) * rsqrt(VdotV);

    // Use saturate(x) instead of max(x,0.f) because that is faster
	return saturate(NdotV - 0.6) * saturate(Falloff(VdotV, Params));
}

void AccumulateAO(
    inout float AO,
    inout float RayPixels,
    float StepSizePixels,
    float2 Direction,
    float2 FullResUV,
    float3 ViewPosition,
    float3 ViewNormal,
    AORadiusParams Params
)
{
	float2 SnappedUV = round(RayPixels * Direction) * InvAORes + FullResUV;

	float3 S = GetViewPos(SnappedUV);

	RayPixels += StepSizePixels;

	AO += ComputeAO(ViewPosition, ViewNormal, S, Params);
}

float ComputeCoarseAO(float2 FullResUV, float3 ViewPosition, float3 ViewNormal, AORadiusParams Params)
{
    // Divide by NUM_STEPS+1 so that the farthest samples are not fully attenuated
	float StepSizePixels = (Params.fRadiusPixels / 4.0) / (NUM_STEPS + 1);
	float4 Rand = 0;
	// Do not use random map
	Rand = float4(1, 0, 1, 1);

	const float Alpha = 2.0 * GFSDK_PI / NUM_DIRECTIONS;
	float SmallScaleAO = 0;
	float LargeScaleAO = 0;

    [unroll]
	for (float DirectionIndex = 0; DirectionIndex < NUM_DIRECTIONS; ++DirectionIndex)
	{
		float Angle = Alpha * DirectionIndex;

        // Compute normalized 2D direction
		float2 Direction = RotateDirection(float2(cos(Angle), sin(Angle)), Rand.xy);

        // Jitter starting sample within the first step
		float RayPixels = (Rand.z * StepSizePixels + 1.0);

        {
			AccumulateAO(SmallScaleAO, RayPixels, StepSizePixels, Direction, FullResUV, ViewPosition, ViewNormal, Params);
		}

        [unroll]
		for (float StepIndex = 1; StepIndex < NUM_STEPS; ++StepIndex)
		{
			AccumulateAO(LargeScaleAO, RayPixels, StepSizePixels, Direction, FullResUV, ViewPosition, ViewNormal, Params);
		}
	}

	float AO = (SmallScaleAO * SmallScaleAOAmount) + (LargeScaleAO * LargeScaleAOAmount);

	AO /= (NUM_DIRECTIONS * NUM_STEPS);

	return AO;
}

float DepthThresholdFactor(float ViewDepth)
{
	return saturate((ViewDepth * ViewDepthThresholdNegInv + 1.0) * ViewDepthThresholdSharpness);
}

float4 main(float2 uv : Texcoord) : SV_Target
{
	if (!HBAO)
		return 0;
	float3 P, Pr, Pl, Pt, Pb;
	P = GetViewPos(uv);

	// Sample neighboring pixels
	Pr = GetViewPos(uv + float2(InvAORes.x, 0));
	Pl = GetViewPos(uv + float2(-InvAORes.x, 0));
	Pt = GetViewPos(uv + float2(0, InvAORes.y));
	Pb = GetViewPos(uv + float2(0, -InvAORes.y));

    // Calculate tangent basis floattors using the minimu difference
	float3 dPdu = MinDiff(P, Pr, Pl);
    //float3 dPdv = MinDiff(P, Pt, Pb) * (AORes.y * InvAORes.x);
	float3 dPdv = MinDiff(P, Pt, Pb);

    // Get the random samples from the noise texture
	float3 viewNormal = normalize(cross(dPdu, dPdv));
	
	AORadiusParams Params = GetAORadiusParams(P.z);
	
	// Early exit if the projected radius is smaller than 1 full-res pixel
    [branch]
	if (Params.fRadiusPixels < 1.0)
	{
		return 1.0;
	}
	
	float ao = 1.0;
	
	ao = ComputeCoarseAO(uv, P, viewNormal, Params);
	
	if (ViewDepthThresholdSharpness != -1.f)
	{
		ao *= DepthThresholdFactor(P.z);
	}
	ao = saturate(1.0 - ao * 2.0);
	return float4(0, 0, 0, 1 - ao);
}