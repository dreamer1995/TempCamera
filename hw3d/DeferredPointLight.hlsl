#define DEFERRED 1
#define IsPBR

Texture2D gbuffer[8] : register(t0);
Texture2D depth : register(t8);

cbuffer PointLightCBuf2 : register(b7) //PS
{
	float3 lightPos2;
	float3 ambient2;
	float3 diffuseColor2;
	float diffuseIntensity2;
	float attConst2;
	float attLin2;
	float attQuad2;
};

cbuffer PointLightCBuf : register(b8) //PS
{
	float3 lightPos3;
	float3 ambient3;
	float3 diffuseColor3;
	float diffuseIntensity3;
	float attConst3;
	float attLin3;
	float attQuad3;
};

cbuffer ShadowTransformCBuf0 : register(b10)
{
	matrix shadowMatrix_M0;
};

cbuffer ShadowTransformCBuf1 : register(b11)
{
	matrix shadowMatrix_M1;
};

cbuffer ShadowTransformCBuf2 : register(b12)
{
	matrix shadowMatrix_M2;
};

#include <PBRHeader.hlsli>
#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "DeferredCommon.hlsli"
#include "ShadingModel.hlsli"

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
	
	if (lightCount > 0u)
	{
#ifndef NoShadow
		float4 shadowCubeWorldPos0 = ToCubeShadowWorldSpace(float4(gBuffer.worldPos, 1.0f), shadowMatrix_M0);
		shadowLevel = CubeShadow(shadowCubeWorldPos0, smap0);
#endif
        //shadowLevel = 1.0f;
		if (shadowLevel != 0.0f)
		{
			EncodePLightData(litData, diffuseColor * diffuseIntensity, lightPos - gBuffer.worldPos, attConst, attLin, attQuad);
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
	}
    
	if (lightCount > 1u)
	{
#ifndef NoShadow
		float4 shadowCubeWorldPos1 = ToCubeShadowWorldSpace(float4(gBuffer.worldPos, 1.0f), shadowMatrix_M1);
		shadowLevel = CubeShadow(shadowCubeWorldPos1, smap1);
#endif
		if (shadowLevel != 0.0f)
		{
			EncodePLightData(litData, diffuseColor2 * diffuseIntensity2, lightPos2 - gBuffer.worldPos, attConst2, attLin2, attQuad2);
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
	}
    
	if (lightCount > 2u)
	{
#ifndef NoShadow
		float4 shadowCubeWorldPos2 = ToCubeShadowWorldSpace(float4(gBuffer.worldPos, 1.0f), shadowMatrix_M2);
		shadowLevel = CubeShadow(shadowCubeWorldPos2, smap2);
#endif
		if (shadowLevel != 0.0f)
		{
			EncodePLightData(litData, diffuseColor3 * diffuseIntensity3, lightPos3 - gBuffer.worldPos, attConst3, attLin3, attQuad3);
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
	}

    // final color = attenuate diffuse & ambient by diffuse texture color and add specular reflected
	outCol = diffuseLighting + specularLighting;

	return float4(outCol, 1);
}