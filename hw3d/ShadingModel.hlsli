#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "LightVectorData.hlsli"

#define ShadingModel_UnLit 0
#define ShadingModel_Phong 1
#define ShadingModel_PBR 2
#define ShadingModel_Liquid 3
#define ShadingModel_Toon 4

struct LightingResult
{
	float3 diffuseLighting;
	float3 specularLighting;
};

struct GBuffer
{
	uint shadingModelID;
	float3 baseColor;
	float3 normal;
#ifdef IsPhong
	float3 specularColor;
	float specularWeight;
	float specularGloss;
#endif
#ifdef IsPBR
	float roughness;
	float metallic;
	float AO;
	float specular;
#endif
};

struct LightData
{
	float3 pointPos;
	float3 pointAmbient;
	float3 pointIrradiance;
	float pointAtt;
	float3 directionalDirection;
	float3 directionalIrradiance;
	float3 pointvToL;
	float3 pointdirToL;
	float pointdistToL;
	float3 directionalDirToL;
};
void BxDF(inout LightingResult litRes, GBuffer gBuffer, LightData litData)
{
	switch (gBuffer.shadingModelID)
	{
	case ShadingModel_Phong:
		PhongShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData);
		break;
	case ShadingModel_PBR:
		PBRShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData);
		break;
	case ShadingModel_Liquid:
		phongShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData);
		break;
	case ShadingModel_Toon:
		ToonShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData);
		break;
	default:
		litRes.diffuseLighting += 0;
		litRes.specularLighting += 0;
	}

}

void PhongShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData)
{

}

void PBRShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData)
{

}

void LiquidShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData)
{

}

void ToonShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData)
{

}
