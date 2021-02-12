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

float4 main(float2 uv : Texcoord, float3 viewRay : Texcoord1) : SV_Target
{
	float3 outCol = 0;

	GBuffer gBuffer;
	DecodeGBuffer(gbuffer, gBuffer, uv);
	float3 realworldpos = DecodeNormal(gBuffer.CustomData0.xyz) * 1000.f;
	float clipZ = depth.SampleLevel(splr, uv, 0).r;
	float4 clipSpacePosition = float4(uv * 2 - 1, clipZ, 1.0f);
	
	float4 viewSpacePosition = mul(clipSpacePosition, matrix_I_P);

    // Perspective division
	viewSpacePosition /= viewSpacePosition.w;

	float4 worldSpacePosition = mul(viewSpacePosition, matrix_I_V);
	//float3 viewRay = input.ViewRay.xyz;
	viewRay = float3(viewRay.xy / viewRay.z, 1.0f);
	float ProjectionA = cameraFNPlane.x / (cameraFNPlane.x - cameraFNPlane.y);
	float ProjectionB = (-cameraFNPlane.x * cameraFNPlane.y) / (cameraFNPlane.x - cameraFNPlane.y);
	float linearDepth = ProjectionB / (clipZ - ProjectionA);
	float3 positionVS = viewRay * linearDepth;
	float3 worldpos = positionVS.xyz + cameraPos;
	return float4(worldpos, 1);

}