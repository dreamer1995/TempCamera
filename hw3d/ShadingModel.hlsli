struct LightingResult
{
	float3 diffuseLighting;
	float3 specularLighting;
};

struct GBuffer
{
	uint shadingModelID;
	float3 worldPos;
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
	float3 pointIrradiance;
	float3 pointDirToL;
	float pointAtten;
	float3 directionalIrradiance;
	float3 directionalDirToL;
};

#include "BxDF.hlsli"

void BxDF(inout LightingResult litRes, GBuffer gBuffer, LightData litData, float3 V, const float shadowLevel)
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
		LiquidShading(litRes, gBuffer, litData, V);
		break;
	case ShadingModel_Toon:
		ToonShading(litRes, gBuffer, litData, V);
		break;
	default:
		litRes.diffuseLighting += 0;
		litRes.specularLighting += 0;
		break;
	}
}

void EncodeLightData(out LightData litData, float3 worldPos)
{
	litData.pointIrradiance = diffuseColor * diffuseIntensity;
	float3 vToL = lightPos - worldPos;
	float distToL = length(vToL);
	litData.pointDirToL = vToL / distToL;
	// attenuation
	litData.pointAtten = Attenuate(attConst, attLin, attQuad, distToL);
	litData.directionalIrradiance = DdiffuseColor * DdiffuseIntensity;
	litData.directionalDirToL = direction;
}

void DecodeGBuffer(MaterialShadingParameters matParams, out GBuffer gBuffer)
{
	gBuffer.shadingModelID = matParams.shadingModelID;
	gBuffer.worldPos = matParams.worldPos;
	gBuffer.baseColor = matParams.baseColor;;
	gBuffer.normal = matParams.normal;
#ifdef IsPhong
	gBuffer.specularColor = matParams.specularColor;
	gBuffer.specularWeight = matParams.specularWeight;
	gBuffer.specularGloss = matParams.specularGloss;
#endif
#ifdef IsPBR
	gBuffer.roughness = matParams.roughness;
	gBuffer.metallic = matParams.metallic;
	gBuffer.AO = matParams.AO;
	gBuffer.specular = matParams.specular;
#endif
}