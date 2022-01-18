Texture2D transmittanceLUT : register(t0);
Texture2D scatteringLUT : register(t1);
SamplerState splr;

#define MULTISCATAPPROX_ENABLED 1

#include "Constants.hlsli"
#include "SkyAtmosphereCommon.hlsli"

struct RayMarchPixelOutputStruct
{
	float4 Luminance : SV_TARGET0;
#if COLORED_TRANSMITTANCE_ENABLED
	float4 Transmittance	: SV_TARGET1;
#endif
};

struct VSOut
{
	float2 uv : Texcoord;
	nointerpolation uint sliceId : SLICEINDEX;
};

float4 main(VSOut IN) : SV_TARGET
{
	return float4(IN.uv, 0, 1);
}