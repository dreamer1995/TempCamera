Texture2D sceneColor : register(t0);
Texture2D depth : register(t8);
SamplerState splr;

static const float PI = 3.14159265359;

#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "DeferredCommon.hlsli"

//cbuffer CBufProperties : register(b10)
//{
//	int numSteps;
//	float mie;
//	bool enableDitherSteps;
//	float4 scaledScreenInfo;
//};

//float RadicalInverse_VdC(uint bits)
//{
//	bits = (bits << 16u) | (bits >> 16u);
//	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
//	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
//	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
//	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
//	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
//}

//// Mie scaterring approximated with Henyey-Greenstein phase function.
//float ComputeScattering(float lightDotView)
//{
//	float result = 1.0f - mie * mie;
//	result /= (4.0f * PI * pow(1.0f + mie * mie - (2.0f * mie) * lightDotView, 1.5f));
//	return result;
//} 
  
RWStructuredBuffer<float4> g_OutBuff;  
  
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float2 uv = (dispatchThreadID.xy + 0.5f) * screenInfo.zw;
	float4 color = sceneColor.SampleLevel(splr, uv, 0);
	int buffIndex = dispatchThreadID.y * screenInfo.x + dispatchThreadID.x;

	g_OutBuff[buffIndex] = color;
}  
