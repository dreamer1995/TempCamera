#include <PBRHeader.hlsli>
#include "Constants.hlsli"
#include "Algorithms.hlsli"

#define IsPBR

Texture2D tex;
Texture2D nmap : register(t1);

cbuffer ObjectCBuf : register(b10)
{
	float3 baseColor;
	float roughness;
	float metallic;
	bool useNormalMap;
	float normalMapWeight;
};

struct PSIn {
	float3 worldPos : Position;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 binormal : Binormal;
	float2 uv : Texcoord;
	float4 shadowHomoPos : ShadowPosition0;
	float4 shadowCubeWorldPos0 : ShadowPosition1;
	float4 shadowCubeWorldPos1 : ShadowPosition2;
	float4 shadowCubeWorldPos2 : ShadowPosition3;
};

struct MaterialShadingParameters
{
	uint shadingModelID;
	float3 worldPos;
	float3 baseColor;
	float3 normal;
	float roughness;
	float metallic;
	float AO;
	float specular;
};

void GetMaterialParameters(out MaterialShadingParameters matParams, PSIn IN)
{
	matParams.shadingModelID = ShadingModel_PBR;
	matParams.worldPos = IN.worldPos;
	matParams.baseColor = baseColor;
	float3 normal = normalize(IN.normal);
	if (useNormalMap)
	{
		const float3 mappedNormal = MapNormal(normalize(IN.tangent), normalize(IN.binormal), normal , IN.uv, nmap, splr);
		normal = lerp(normal, mappedNormal, normalMapWeight);
	}
	matParams.normal = normalize(normal);
	matParams.roughness = roughness;
	matParams.metallic = metallic;
	matParams.AO = 1.0f;
	matParams.specular = 1.0f;
}

#include "ForwardRenderingTrunk.hlsli"