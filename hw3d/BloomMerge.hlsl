#include "Constants.hlsli"

Texture2D bloomColor : register(t0);
SamplerState splr;

float4 main(float2 uv : Texcoord) : SV_Target
{
	if (!HDR)
		return 0;
	float3 color = bloomColor.SampleLevel(splr, uv, 0).rgb;
	
	return float4(color, 1);
}