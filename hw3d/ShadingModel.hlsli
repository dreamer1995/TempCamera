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
	float4 CustomData0;
};

struct LightData
{
	float3 irradiance;
	float3 dirToL;
};

#include "BxDF.hlsli"

void BxDF(out LightingResult litRes, GBuffer gBuffer, LightData litData, float3 V, float shadowLevel)
{
	litRes.diffuseLighting = 0.0f;
	litRes.specularLighting = 0.0f;
	switch (gBuffer.shadingModelID)
	{
	case ShadingModel_Phong:
#ifdef IsPhong
		PhongShading(litRes, gBuffer, litData, V);
#endif
		break;
	case ShadingModel_PBR:
#ifdef IsPBR
		PBRShading(litRes, gBuffer, litData, V);
#endif
		break;
	case ShadingModel_Liquid:
#ifdef IsPBR
		LiquidShading(litRes, gBuffer, litData, V);
#endif
		break;
	case ShadingModel_Toon:
		ToonShading(litRes, gBuffer, litData, V);
		break;
	}
}

void BxDF_Ambient(out float3 ambientLighting, GBuffer gBuffer, float3 V)
{
	ambientLighting = 0.0f;
	switch (gBuffer.shadingModelID)
	{
	case ShadingModel_Phong:
#ifdef IsPhong
		PhongAmbientShading(ambientLighting, gBuffer);
#endif
		break;
	case ShadingModel_PBR:
	case ShadingModel_Liquid:
#ifdef IsPBR
		PBRAmbientShading(ambientLighting, gBuffer, V);
#endif
		break;
	}
}

void EncodeLightData(out LightData litData, float3 irradiance, float3 vToL, bool IsDirectional)
{
	litData.irradiance = irradiance;
	if (IsDirectional)
	{
		litData.dirToL = vToL;
	}
	else
	{
		float distToL = length(vToL);
		litData.dirToL = vToL / distToL;
		// attenuation
		litData.irradiance *= Attenuate(attConst, attLin, attQuad, distToL);
	}
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
	if (gBuffer.shadingModelID == ShadingModel_Liquid)
	{
#ifdef PixelsWave
		gBuffer.CustomData0 = float4(matParams.causticsColor, 0.0f);
#endif
	}
	else
		gBuffer.CustomData0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
}