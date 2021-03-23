Texture2D volumeColor : register(t0);
Texture2D depth : register(t8);
SamplerState splr;

static const float PI = 3.14159265359;

#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "DeferredCommon.hlsli"

float4 main(float2 uv : Texcoord) : SV_Target
{
	if (!volumetricRendering)
		return float4(volumeColor.SampleLevel(splr, uv, 0).rgb, 1);
	
	float upSampledDepth = depth.SampleLevel(splr, uv, 0).r;

	float3 color = 0.0f;
	float totalWeight = 0.0f;

	// Select the closest downscaled pixels.

	int xOffset = uv.x % 2 == 0 ? -screenInfo.z : screenInfo.z;
	int yOffset = uv.y % 2 == 0 ? -screenInfo.w : screenInfo.w;

	int2 offsets[] = {int2(0, 0),
	int2(0, yOffset),
	int2(xOffset, 0),
	int2(xOffset, yOffset)};

	for (int i = 0; i < 4; i ++)
	{
		float3 downscaledColor = volumeColor.SampleLevel(splr, uv + offsets[i], 0).rgb;

		float downscaledDepth = depth.SampleLevel(splr, uv + offsets[i], 0).rgb;

		float currentWeight = 1.0f;
		currentWeight *= max(0.0f, 1.0f - (0.05f) * abs(downscaledDepth - upSampledDepth));

		color += downscaledColor * currentWeight;
		totalWeight += currentWeight;
	}

	float3 volumetricLight;
	const float epsilon = 0.0001f;
	volumetricLight.xyz = color/(totalWeight + epsilon);

	return float4(volumetricLight.xyz, 1.0f);
}