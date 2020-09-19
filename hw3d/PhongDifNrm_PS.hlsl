#include "Constants.hlsli"

#include "LightVectorData.hlsli"
#include "Algorithms.hlsli"

cbuffer ObjectCBuf : register(b10)
{
    float3 specularColor;
    float specularWeight;
    float specularGloss;
    bool useNormalMap;
    float normalMapWeight;
};

Texture2D tex;
Texture2D nmap : register(t2);
SamplerState splr;

struct PSIn
{
    float3 worldPos : Position;
    float3 normal : Normal;
    float3 tan : Tangent;
    float3 binor : Binormal;
    float2 tc : Texcoord;
};

float4 main(PSIn i) : SV_Target
{
    // normalize the mesh normal
    float3 normal = normalize(i.normal);
    // replace normal with mapped if normal mapping enabled
    if (useNormalMap)
    {
        const float3 mappedNormal = MapNormal(normalize(i.tan), normalize(i.binor), normal , i.tc, nmap, splr);
        normal = lerp(normal, mappedNormal, normalMapWeight);
    }
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