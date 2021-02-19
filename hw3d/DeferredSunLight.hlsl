#define DEFERRED 1
#define IsPBR

Texture2D gbuffer[8] : register(t0);
Texture2D depth : register(t8);

#include <PBRHeader.hlsli>
#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "DeferredCommon.hlsli"
#include "ShadingModel.hlsli"
#include "ConstantsPS.hlsli"

float4 main(float2 uv : Texcoord) : SV_Target
{
	float3 outCol = 0;

	GBuffer gBuffer;
	DecodeGBuffer(gbuffer, gBuffer, uv);
	gBuffer.normal = normalize(gBuffer.normal);
	float3 V = normalize(cameraPos - gBuffer.worldPos);

    // Dynamic Lighting
	float3 diffuseLighting = 0.0f;
	float3 specularLighting = 0.0f;

	LightingResult litRes;
	LightData litData;
	float shadowLevel = 1.0f;
	
#ifndef NoShadow
	float4 shadowHomoPos = ToShadowHomoSpace(float4(gBuffer.worldPos, 1.0f), shadowMatrix_VP);
	shadowLevel = Shadow(shadowHomoPos, smap);
#endif
    //shadowLevel = 1.0f;
	if (shadowLevel != 0.0f)
	{
		EncodeDLightData(litData, DdiffuseColor * DdiffuseIntensity, direction);
		BxDF(litRes, gBuffer, litData, V, shadowLevel);
        // scale by shadow level
		litRes.diffuseLighting *= shadowLevel;
		litRes.specularLighting *= shadowLevel;
	}
	else
	{
		litRes.diffuseLighting = litRes.specularLighting = 0.0f;
	}
	diffuseLighting += litRes.diffuseLighting;
	specularLighting += litRes.specularLighting;

    // Ambient Lighting
	float3 ambientLighting;
	BxDF_Ambient(ambientLighting, gBuffer, V, ambient);

    // final color = attenuate diffuse & ambient by diffuse texture color and add specular reflected
	outCol = diffuseLighting + specularLighting + ambientLighting;

	outCol = EncodeGamma(outCol);

	return float4(outCol, 1);
}