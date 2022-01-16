SamplerState splr;

#include "Constants.hlsli"
#include "SkyAtmosphereCommon.hlsli"

float4 main(float2 uv : Texcoord) : SV_Target
{
	float2 pixPos = uv * float2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
	AtmosphereParameters Atmosphere = GetAtmosphereParameters();

	// Compute camera position from LUT coords
	//float2 uv = (pixPos) / float2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
	float viewHeight;
	float viewZenithCosAngle;
	UvToLutTransmittanceParams(Atmosphere, viewHeight, viewZenithCosAngle, uv);

	//  A few extra needed constants
	float3 WorldPos = float3(0.0f, 0.0f, viewHeight);
	float3 WorldDir = float3(0.0f, sqrt(1.0f - viewZenithCosAngle * viewZenithCosAngle), viewZenithCosAngle);

	const bool ground = false;
	const float SampleCountIni = 40.0f; // Can go a low as 10 sample but energy lost starts to be visible.
	const float DepthBufferValue = -1.0f;
	const bool VariableSampleCount = false;
	const bool MieRayPhase = false;
	float3 transmittance = exp(-IntegrateScatteredLuminance(pixPos, WorldPos, WorldDir, direction.xzy,
		Atmosphere, ground, SampleCountIni, DepthBufferValue, VariableSampleCount, MieRayPhase).OpticalDepth);

	// Opetical depth to transmittance
	return float4(transmittance, 1.0f);
}
