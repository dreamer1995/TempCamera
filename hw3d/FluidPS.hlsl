#include "Constants.hlsli"
#include <PBRHeader.hlsli>
#include "Algorithms.hlsli"

#define IsPBR

cbuffer ObjectCBuf2 : register(b10)
{
	float speed;
	float roughness;
	float flatten1;
	float flatten2;
	bool normalMapEnabled;
};

cbuffer ObjectCBuf : register(b11)
{
	float metallic;
	float tilling;
	float depth;
};

Texture2D rmap;
Texture2D mnmap : register(t1);
Texture2D snmap : register(t2);
Texture2D gmap : register(t3);
Texture2D hmap : register(t4);
Texture2D caumap : register(t5);

#include "WaveCommon.hlsli"

struct PSIn {
	float3 worldPos : Position;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 binormal : Binormal;
	float2 uv : Texcoord;
	float3 outScattering : Position1;
	float3 inScattering : Position2;
	float4 shadowHomoPos : ShadowPosition;
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
	matParams.shadingModelID = ShadingModel_Liquid;
	matParams.worldPos = IN.worldPos;
	float fRoughness = Motion_4WayChaos(rmap, IN.uv * 4.0f, 0.05f);
	// const float variationAmount;
	fRoughness += 0.5f;
	const float variationSharpness = 13.0511389f;
	fRoughness = pow(fRoughness, variationSharpness) * variationSharpness;
	fRoughness = saturate(fRoughness);
	fRoughness = roughness * lerp(0.164602f, 0.169983f, fRoughness);
	matParams.roughness = fRoughness;

	// sample normal from map if normal mapping enabled	
	if (normalMapEnabled)
	{
		float3 mediumWaves = lerp(Motion_4WayChaos_Normal(mnmap, IN.uv * tilling, speed), float3(0.5f, 0.5f, 1.0f), flatten1);
		float3 smallWaves = lerp(Motion_4WayChaos_Normal(snmap, IN.uv * tilling, speed), float3(0.5f, 0.5f, 1.0f), flatten2);
		float3 mixWaves = (mediumWaves + smallWaves) * 0.5f;
		float waveletNormalStrength = 0.034862f;
		float3 wavelet = lerp(float3(0.5f, 0.5f, 1.0f), Motion_4WayChaos_Normal(snmap, IN.uv * 0.5f, 0.40708f), waveletNormalStrength);

		float3 bumpNormal = lerp(mixWaves, wavelet, fRoughness);
		bumpNormal = bumpNormal * 2.0f - 1.0f;
		bumpNormal = (bumpNormal.x * normalize(IN.tangent)) + (bumpNormal.y * normalize(IN.binormal)) + (bumpNormal.z * normalize(IN.normal));
		matParams.normal = normalize(bumpNormal);
	}
	else
	{
		matParams.normal = normalize(IN.normal);
	}
	
	// float fresnel = 1 - NdotV;
	//float3 albedo = lerp(pow(float3(0.018450f, 0.045000f, 0.042473f), 2.2f),
	//					 pow(float3(0.162565f, 0.271166f, 0.325000f), 2.2f),
	//					 fresnel);

	//const float3 statVDir = normalize(cameraPos - (float3)mul(float4(0.0f, 0.0f, 0.0f, 1.0f), matrix_M2W));
	const float depthmap = (IN.worldPos.z * 0.1f + 1.0f) * 0.5f;

	const float t = lerp(0.225f, 0.465f, max(dot(matParams.normal, -cameraDir), 0.0f));
	const float3 Rv = lerp(cameraDir, -matParams.normal, t);
	const float depthR = depth + IN.worldPos.y;
	const float2 distUV = UVRefractionDistorted(Rv, IN.uv, depthR * depthmap);
	const float2 subDistUV = UVRefractionDistorted(Rv, IN.uv, depthR * depthmap * 0.5f);
	float3 albedo = DecodeGamma(gmap.Sample(splr, distUV).rgb);
	albedo += caumap.Sample(splr, subDistUV * tilling).rgb * saturate(1 - depthR * depthmap);
	matParams.baseColor = albedo * (IN.outScattering + IN.inScattering);
	
	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	matParams.metallic = saturate(metallic * depthR * depthmap);
	matParams.AO = 1.0f;
	matParams.specular = 1.0f;
}

#include "ForwardRenderingTrunk.hlsli"
