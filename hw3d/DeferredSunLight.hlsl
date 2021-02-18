#define DEFERRED 1
#define IsPBR
#include <PBRHeader.hlsli>
#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "DeferredCommon.hlsli"
#include "ShadingModel.hlsli"
#include "ConstantsPS.hlsli"

Texture2D gbuffer[8] : register(t0);
Texture2D depth : register(t8);

float4 main(float2 uv : Texcoord) : SV_Target
{
	float3 outCol = 0;

	GBuffer gBuffer;
	DecodeGBuffer(gbuffer, gBuffer, uv);
	
	float sceneZ = depth.SampleLevel(splrClamp, uv, 0).x;
	sceneZ = ConvertToLinearDepth(sceneZ);	
	float3 D = CalcHomogeneousPos(sceneZ, uv);
	float3 worldPos = cameraPos.xyz + D;
	
	return float4(worldPos, 1);
}