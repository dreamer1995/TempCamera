Texture2D volumeColor : register(t0);
Texture2D volumeColorX : register(t1);
Texture2D depth : register(t8);
SamplerState splr;

static const float PI = 3.14159265359;

cbuffer Control : register(b10)
{
	bool horizontal;
}

#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "DeferredCommon.hlsli"

float4 main(float2 uv : Texcoord) : SV_Target
{
	if (horizontal)
	{
		return float4(volumeColor.SampleLevel(splr, uv, 0).rgb, 1.0f);
	}
	else
	{
		float3 OUT = volumeColorX.Sample(splr, uv).rgb;
		
		return float4(OUT, 1.0f);
	}
		
}