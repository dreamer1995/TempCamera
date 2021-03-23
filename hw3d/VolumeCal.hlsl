Texture2D depth : register(t8);
SamplerState splr;

static const float PI = 3.14159265359;

#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "DeferredCommon.hlsli"

cbuffer CBufProperties : register(b10)
{
	int numSteps;
	float mie;
	bool ditherSteps;
};

// Mie scaterring approximated with Henyey-Greenstein phase function.
float ComputeScattering(float lightDotView)
{
	float result = 1.0f - mie * mie;
	result /= (4.0f * PI * pow(1.0f + mie * mie - (2.0f * mie) * lightDotView, 1.5f));
	return result;
}

float4 main(float2 uv : Texcoord) : SV_Target
{
	//if (!volumetricRendering)
	//	return 0;
	float sceneZ = depth.SampleLevel(splr, uv, 0).x;
	sceneZ = ConvertToLinearDepth(sceneZ);	
	float3 D = CalcHomogeneousPos(sceneZ, uv);
	float3 worldPos = cameraPos.xyz + D;

	float3 startPosition = cameraPos;

	float3 rayVector = worldPos - startPosition;

	float rayLength = length(rayVector);
	float3 rayDirection = rayVector / rayLength;

	float stepLength = rayLength / numSteps;

	float3 step = rayDirection * stepLength;

	float3 currentPosition = startPosition;

	float3 accumFog = 0.0f;

	float3 ditherPattern[4] = {{0.0f, 0.5f, 0.125f},
						   {0.75f, 0.22f, 0.875f},
						   {0.1875f, 0.6875f, 0.0625f},
						   {0.9375f, 0.4375f, 0.8125f}};
	
	for (int i = 0; i < numSteps; i++)
	{
		float4 shadowHomoPos = ToShadowHomoSpace(float4(currentPosition, 1.0f), shadowMatrix_VP);
		const float3 spos = shadowHomoPos.xyz / shadowHomoPos.w;
		float shadowMapValue = smap.SampleLevel(splr, spos.xy, 0).r;

		if (shadowMapValue >= spos.z)
			accumFog += ComputeScattering(dot(rayDirection, direction)) * DdiffuseColor * DdiffuseIntensity;
				
		// Offset the start position.
		if (ditherSteps)
			currentPosition += step * ditherPattern[i % 4];
		else
			currentPosition += step;
	}
	accumFog /= numSteps;

	return float4(accumFog, 1);
}