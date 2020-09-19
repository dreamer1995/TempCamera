#include "Constants.hlsli"

#include "LightVectorData.hlsli"
#include "Algorithms.hlsli"

cbuffer ObjectCBuf : register(b10)
{
    float3 specularColor;
    float specularWeight;
    float specularGloss;
};

Texture2D tex;
SamplerState splr;

struct PSIn {
    float3 worldPos : Position;
    float3 normal : Normal;
    float2 tc : Texcoord;
};

float4 main(PSIn i) : SV_Target
{
    // renormalize interpolated normal
    float3 normal = normalize(i.normal);
	// fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(lightPos, i.worldPos);
	// attenuation
    const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
	// diffuse
    float3 diffuse = Diffuse(lv.irradiance, att, lv.dirToL, normal);
    diffuse += Diffuse(DdiffuseColor * DdiffuseIntensity, 1.0f, direction, normal);
	// specular
    float3 specular = Speculate(cameraPos, i.worldPos, lv.dirToL, lv.irradiance * specularColor, specularWeight, normal, att, specularGloss);
    specular += Speculate(cameraPos, i.worldPos, direction, DdiffuseColor * DdiffuseIntensity * specularColor, specularWeight, normal, 1.0f, specularGloss);
	// final color
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, i.tc).rgb + specular), 1.0f);
}