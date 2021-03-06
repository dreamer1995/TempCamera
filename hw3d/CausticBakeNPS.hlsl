#include "Constants.hlsli"
#include <PBRHeader.hlsli>

#define PixelsWave

cbuffer ObjectCBuf : register(b10)
{
	float speed;
	float roughness;
	float flatten1;
	float flatten2;
	bool normalMapEnabled;
};

Texture2D rmap;
Texture2D mnmap : register(t1);
Texture2D snmap : register(t2);

#include "WaveCommon.hlsli"

struct PSIn {
	float3 worldPos : Position;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 binormal : Binormal;
	float2 uv : Texcoord;
};

float4 main(PSIn i) : SV_Target
{
	float fRoughness = Motion_4WayChaos(rmap, i.uv * 4.0f, 0.05f);
	float variationAmount;
	fRoughness += 0.5f;
	float variationSharpness = 13.0511389f;
	fRoughness = pow(fRoughness, variationSharpness) * variationSharpness;
	fRoughness = saturate(fRoughness);
	fRoughness = roughness * lerp(0.164602f, 0.169983f, fRoughness);
	// sample normal from map if normal mapping enabled	
	if (normalMapEnabled)
	{
		float3 mediumWaves = lerp(Motion_4WayChaos_Normal(mnmap, i.uv, speed), float3(0.5f, 0.5f, 1.0f), flatten1);
		float3 smallWaves = lerp(Motion_4WayChaos_Normal(snmap, i.uv, speed), float3(0.5f, 0.5f, 1.0f), flatten2 + 0.65f);
		float3 mixWaves = (mediumWaves + smallWaves) * 0.5f;
		float waveletNormalStrength = 0.034862f;
		float3 wavelet = lerp(float3(0.5f, 0.5f, 1.0f), Motion_4WayChaos_Normal(snmap, i.uv * 0.5f, 0.40708f), waveletNormalStrength);

		float3 bumpNormal = lerp(mixWaves, wavelet, fRoughness);
		bumpNormal = bumpNormal * 2.0f - 1.0f;
		bumpNormal = (bumpNormal.x * normalize(i.tangent)) + (bumpNormal.y * normalize(i.binormal)) + (bumpNormal.z * normalize(i.normal));
		i.normal = normalize(bumpNormal);
	}
	else
	{
		i.normal = normalize(i.normal);
	}

	return float4((i.normal + 1.0f) * 0.5f, 1.0f);

}