#include <PBRHeader.hlsli>
#include "Constants.hlsli"
#include "Algorithms.hlsli"

#define IsPBR

Texture2D tex;
Texture2D mramap : register(t1);
Texture2D nmap : register(t2);

cbuffer ObjectCBuf : register(b10)
{
	bool enableAbedoMap;
	bool enableMRAMap;
	bool enableNormalMap;
	bool useAbedoMap;
	float3 materialColor;
	bool useMetallicMap;
	bool useRoughnessMap;
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
	float3 abedo = materialColor;
	if (enableAbedoMap)
	{
		if (useAbedoMap)
		{
			float4 basecolor = tex.Sample(splr, IN.uv);
		#ifdef AlphaTest
			// bail if highly translucent
			clip(basecolor.a < 0.1f ? -1 : 1);
		#endif
			abedo *= DecodeGamma(basecolor.rgb);
		}
	}
	matParams.baseColor = abedo;

	float fmetallic = metallic;
	float froughness = roughness;
	float AO = 1.0f;
	if (enableMRAMap)
	{
		float3 MRA = mramap.Sample(splr, IN.uv).rgb;
		if (useMetallicMap)
		{
			fmetallic *= MRA.x;
		}
		if (useRoughnessMap)
		{
			froughness *= MRA.y;
		}
		AO *= MRA.z;
	}
	matParams.roughness = froughness;
	matParams.metallic = fmetallic;	
	matParams.AO = AO;
	matParams.specular = 1.0f;

	float3 normal = normalize(IN.normal);
	if (enableNormalMap)
	{
		if (useNormalMap)
		{
			const float3 mappedNormal = MapNormal(normalize(IN.tangent), normalize(IN.binormal), normal, IN.uv, nmap, splr);
			normal = lerp(normal, mappedNormal, normalMapWeight);
			normal = normalize(normal);
		}
	}
#ifdef AlphaTest
	// flip normal when backface
	if (dot(normal, normalize(cameraPos - IN.worldPos)) <= 0.0f)
	{
		normal = -normal;
	}
#endif
	matParams.normal = normal;
}

#include "ForwardRenderingTrunk.hlsli"