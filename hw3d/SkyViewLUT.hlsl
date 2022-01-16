//StructuredBuffer<float4> sceneColor : register(t0);
Texture2D transmittanceLUT : register(t0);
Texture2D scatteringLUT : register(t1);
SamplerState splr;

#include "Constants.hlsli"
#include "SkyAtmosphereCommon.hlsli"

float4 main(float2 uv : Texcoord) : SV_Target
{
	float2 pixPos = uv * float2(192.0f, 108.0f);
	AtmosphereParameters Atmosphere = GetAtmosphereParameters();

	float3 ClipSpace = float3(uv * float2(2.0f, -2.0f) - float2(1.0f, -1.0f), 1.0f);
	float4 HPos = mul(matrix_I_VP, float4(ClipSpace, 1.0));

	float3 WorldDir = normalize(HPos.xyz / HPos.w - cameraPos);
	float3 WorldPos = cameraPos + float3(0, Atmosphere.BottomRadius, 0);

	float viewHeight = length(WorldPos);

	float viewZenithCosAngle;
	float lightViewCosAngle;
	UvToSkyViewLutParams(Atmosphere, viewZenithCosAngle, lightViewCosAngle, viewHeight, uv);


	float3 SunDir;
	{
		float3 UpVector = WorldPos / viewHeight;
		float sunZenithCosAngle = dot(UpVector, direction);
		SunDir = normalize(float3(sqrt(1.0f - sunZenithCosAngle * sunZenithCosAngle), sunZenithCosAngle, 0.0f));
	}



	WorldPos = float3(0.0f, viewHeight, 0.0f);

	float viewZenithSinAngle = sqrt(1 - viewZenithCosAngle * viewZenithCosAngle);
	WorldDir = float3(
		viewZenithSinAngle * lightViewCosAngle,
		viewZenithCosAngle,
		viewZenithSinAngle * sqrt(1.0f - lightViewCosAngle * lightViewCosAngle));


	// Move to top atmospehre
	if (!MoveToTopAtmosphere(WorldPos, WorldDir, Atmosphere.TopRadius))
	{
		// Ray is not intersecting the atmosphere
		return float4(0, 0, 0, 1);
	}

	const bool ground = false;
	const float SampleCountIni = 30;
	const float DepthBufferValue = -1.0;
	const bool VariableSampleCount = true;
	const bool MieRayPhase = true;
	SingleScatteringResult ss = IntegrateScatteredLuminance(pixPos, WorldPos, WorldDir, SunDir, Atmosphere, ground, SampleCountIni, DepthBufferValue, VariableSampleCount, MieRayPhase);

	float3 L = ss.L;

	return float4(L, 1);
}