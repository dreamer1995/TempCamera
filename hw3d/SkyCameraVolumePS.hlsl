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

float4 main(PSIn IN) : SV_Target
{
	return float4(IN.uv, 0, 1);
}