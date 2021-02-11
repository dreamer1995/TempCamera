#define DEFERRED 1
#define IsPBR
#include "DeferredCommon.hlsli"
#include <PBRHeader.hlsli>
#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "ShadingModel.hlsli"
#include "ConstantsPS.hlsli"

Texture2D gbuffer[8] : register(t0);
Texture2D depth : register(t8);

float4 main(float2 uv : Texcoord) : SV_Target
{
	float3 outCol = 0;

	GBuffer gBuffer;
	DecodeGBuffer(gbuffer, gBuffer, uv);
	return float4(gBuffer.normal, 1);

}