StructuredBuffer<float4> sceneColor : register(t0);
SamplerState splr;

#include "Constants.hlsli"

float4 main(float2 uv : Texcoord) : SV_Target
{
	int index = round((uv.y * screenInfo.y - 0.5f) * screenInfo.x + (uv.x * screenInfo.x - 0.5f));
	
	float3 color = sceneColor[index].rgb;
	
	return float4(color, 1);
}