#include "Constants.hlsli"
#include "Algorithms.hlsli"

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
#ifdef EnableAlpha;
	float alpha;
#endif
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
	float3 pointIrradiance;
	float3 pointDirToL;
	float pointAtten;
	float3 directionalIrradiance;
	float3 directionalDirToL;
};
void BxDF(inout LightingResult litRes, GBuffer gBuffer, LightData litData, float3 V)
{
	switch (gBuffer.shadingModelID)
	{
	case ShadingModel_Phong:
		PhongShading(litRes, gBuffer, litData, V);
		break;
	case ShadingModel_PBR:
		PBRShading(litRes, gBuffer, litData, V);
		break;
	case ShadingModel_Liquid:
		phongShading(litRes, gBuffer, litData, V);
		break;
	case ShadingModel_Toon:
		ToonShading(litRes, gBuffer, litData, V);
		break;
	default:
		litRes.diffuseLighting += 0;
		litRes.specularLighting += 0;
	}

}

void PhongShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData, float3 V)
{

}

void PBRShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData, float3 V)
{

}

void LiquidShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData, float3 V)
{

}

void ToonShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData, float3 V)
{

}
