Texture2D sceneColor : register(t0);
Texture2D depth : register(t8);
SamplerState splr;

static const float PI = 3.14159265359;

#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "DeferredCommon.hlsli"
  
RWTexture2D<float4> OutputTexture : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float2 uv = (dispatchThreadID.xy + 0.5f) * screenInfo.zw;
	float4 color = sceneColor.SampleLevel(splr, uv, 0);

	OutputTexture[dispatchThreadID.xy] = float4(dispatchThreadID.x + 0.5f, dispatchThreadID.y + 0.5f, 0.0f, 1.0f);
}  
