Texture2D transmittanceLUT : register(t0);
Texture2D scatteringLUT : register(t1);
SamplerState splr;

#define MULTISCATAPPROX_ENABLED 1

#include "Constants.hlsli"
#include "SkyAtmosphereCommon.hlsli"

struct PSIn
{
	float2 uv : Texcoord;
	nointerpolation uint sliceId : SV_RenderTargetArrayIndex; //write to a specific slice, it can also be read in the pixel shader.
};

float4 main(PSIn IN) : SV_TARGET0
{
	float2 pixPos = IN.uv * float2(32.0f, 32.0f);
	AtmosphereParameters Atmosphere = GetAtmosphereParameters();

	float3 ClipSpace = float3(IN.uv * float2(2.0f, -2.0f) - float2(1.0f, -1.0f), 0.5f);
	float4 HPos = mul(float4(ClipSpace, 1.0), matrix_I_VP);
	HPos.xyz = HPos.xzy;
	float3 WorldDir = normalize(HPos.xyz / HPos.w - cameraPos.xzy);
	float earthR = Atmosphere.BottomRadius;
	float3 earthO = float3(0.0f, 0.0f, -earthR);
	float3 camPos = cameraPos.xzy + float3(0, 0, earthR);
	float3 SunDir = direction.xzy;
	float3 SunLuminance = 0.0f;

	float Slice = ((float(IN.sliceId) + 0.5f) / AP_SLICE_COUNT);
	Slice *= Slice; // squared distribution
	Slice *= AP_SLICE_COUNT;

	float3 WorldPos = camPos;
	float viewHeight;


	// Compute position from froxel information
	float tMax = AerialPerspectiveSliceToDepth(Slice);
	float3 newWorldPos = WorldPos + tMax * WorldDir;


	// If the voxel is under the ground, make sure to offset it out on the ground.
	viewHeight = length(newWorldPos);
	if (viewHeight <= (Atmosphere.BottomRadius + PLANET_RADIUS_OFFSET))
	{
		// Apply a position offset to make sure no artefact are visible close to the earth boundaries for large voxel.
		newWorldPos = normalize(newWorldPos) * (Atmosphere.BottomRadius + PLANET_RADIUS_OFFSET + 0.001f);
		WorldDir = normalize(newWorldPos - camPos);
		tMax = length(newWorldPos - camPos);
	}
	float tMaxMax = tMax;


	// Move ray marching start up to top atmosphere.
	viewHeight = length(WorldPos);
	if (viewHeight >= Atmosphere.TopRadius)
	{
		float3 prevWorlPos = WorldPos;
		if (!MoveToTopAtmosphere(WorldPos, WorldDir, Atmosphere.TopRadius))
		{
			// Ray is not intersecting the atmosphere
			return float4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		float LengthToAtmosphere = length(prevWorlPos - WorldPos);
		if (tMaxMax < LengthToAtmosphere)
		{
			// tMaxMax for this voxel is not within earth atmosphere
			return float4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		// Now world position has been moved to the atmosphere boundary: we need to reduce tMaxMax accordingly. 
		tMaxMax = max(0.0f, tMaxMax - LengthToAtmosphere);
	}


	const bool ground = false;
	const float SampleCountIni = max(1.0f, float(IN.sliceId + 1.0f) * 2.0f);
	const float DepthBufferValue = -1.0f;
	const bool VariableSampleCount = false;
	const bool MieRayPhase = true;
	SingleScatteringResult ss = IntegrateScatteredLuminance(pixPos, WorldPos, WorldDir, SunDir, Atmosphere, ground, SampleCountIni, DepthBufferValue, VariableSampleCount, MieRayPhase, tMaxMax);


	const float Transmittance = dot(ss.Transmittance, float3(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f));
	return float4(ss.L, 1.0f - Transmittance);
}