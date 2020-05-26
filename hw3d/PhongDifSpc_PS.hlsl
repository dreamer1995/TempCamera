#include "Constants.hlsl"

#include "LightVectorData.hlsl"
#include "Algorithms.hlsl"

cbuffer ObjectCBuf : register(b4)
{
    bool useGlossAlpha;
    bool useSpecularMap;
    float3 specularColor;
    float specularWeight;
    float specularGloss;
};

Texture2D tex;
Texture2D spec;

SamplerState splr;

struct PSIn
{
    float3 worldPos : Position;
    float3 normal : Normal;
    float2 tc : Texcoord;
};

float4 main(PSIn i) : SV_Target
{
    // normalize the mesh normal
    float3 normal = normalize(i.normal);
	// fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(lightPos, i.worldPos);
    // specular parameters
    float specularPowerLoaded = specularGloss;
    const float4 specularSample = spec.Sample(splr, i.tc);
    float3 specularReflectionColor;
    if (useSpecularMap)
    {
        specularReflectionColor = specularSample.rgb;
    }
    else
    {
        specularReflectionColor = specularColor;
    }
    if (useGlossAlpha)
    {
        specularPowerLoaded = pow(2.0f, specularSample.a * 13.0f);
    }
	// attenuation
    const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
	// diffuse light
    float3 diffuse = Diffuse(lv.irradiance, att, lv.dirToL, normal);
    diffuse += Diffuse(DdiffuseColor * DdiffuseIntensity, 1.0f, direction, normal);
    // specular reflected
    float3 specularReflected = Speculate(cameraPos, i.worldPos, lv.dirToL, lv.irradiance * specularReflectionColor,
        specularWeight, normal, att, specularPowerLoaded);
    specularReflected += Speculate(cameraPos, i.worldPos, direction, DdiffuseColor * DdiffuseIntensity * specularReflectionColor,
        specularWeight, normal, 1.0f, specularPowerLoaded);
	// final color = attenuate diffuse & ambient by diffuse texture color and add specular reflected
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, i.tc).rgb + specularReflected), 1.0f);
}