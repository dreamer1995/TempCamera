//StructuredBuffer<float4> sceneColor : register(t0);
Texture2D transmittanceLUT : register(t0);
Texture2D scatteringLUT : register(t1);
SamplerState splr;

#define MULTISCATAPPROX_ENABLED 1

#include "Constants.hlsli"
#include "SkyAtmosphereCommon.hlsli"

float4 main(float2 uv : Texcoord) : SV_Target
{
	float2 pixPos = uv * float2(192.0f, 108.0f);
	AtmosphereParameters Atmosphere = GetAtmosphereParameters();

	float3 ClipSpace = float3(uv * float2(2.0f, -2.0f) - float2(1.0f, -1.0f), 1.0f);
	float4 HPos = mul(float4(ClipSpace, 1.0f), matrix_I_VP);
	HPos.xyz = HPos.xzy;
	
	float3 WorldDir = normalize(HPos.xyz / HPos.w - cameraPos.xzy);
	float3 WorldPos = cameraPos.xzy + float3(0, 0, Atmosphere.BottomRadius);

	float viewHeight = length(WorldPos);

	float viewZenithCosAngle;
	float lightViewCosAngle;
	UvToSkyViewLutParams(Atmosphere, viewZenithCosAngle, lightViewCosAngle, viewHeight, uv);


	float3 SunDir;
	{
		float3 UpVector = WorldPos / viewHeight;
		float sunZenithCosAngle = dot(UpVector, direction.xzy);
		SunDir = normalize(float3(sqrt(1.0f - sunZenithCosAngle * sunZenithCosAngle), 0.0f, sunZenithCosAngle));
	}



	WorldPos = float3(0.0f, 0.0f, viewHeight);

	float viewZenithSinAngle = sqrt(1 - viewZenithCosAngle * viewZenithCosAngle);
	WorldDir = float3(
		viewZenithSinAngle * lightViewCosAngle,
		viewZenithSinAngle * sqrt(1.0 - lightViewCosAngle * lightViewCosAngle),
		viewZenithCosAngle);


	// Move to top atmospehre
	if (!MoveToTopAtmosphere(WorldPos, WorldDir, Atmosphere.TopRadius))
	{
		// Ray is not intersecting the atmosphere
		return float4(0, 0, 0, 1);
	}

	const bool ground = false;
	const float SampleCountIni = 30;
	const float DepthBufferValue = -1.0f;
	const bool VariableSampleCount = true;
	const bool MieRayPhase = true;
	SingleScatteringResult ss = IntegrateScatteredLuminance(pixPos, WorldPos, WorldDir, SunDir, Atmosphere, ground, SampleCountIni, DepthBufferValue, VariableSampleCount, MieRayPhase);

	float3 L = ss.L;

	return float4(L, 1);
}