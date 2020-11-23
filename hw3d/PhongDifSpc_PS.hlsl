#include "Constants.hlsli"

#include "LightVectorData.hlsli"
#include "Algorithms.hlsli"

cbuffer ObjectCBuf : register(b10)
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
    float4 shadowHomoPos : ShadowPosition;
};

float4 main(PSIn i) : SV_Target
{
    float3 diffuse;
    float3 specularReflected;

    const float shadowLevel = Shadow(i.shadowHomoPos);
    if (shadowLevel != 0.0f)
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
        diffuse = Diffuse(lv.irradiance, att, lv.dirToL, normal);
        diffuse += Diffuse(DdiffuseColor * DdiffuseIntensity, 1.0f, direction, normal);
        // specular reflected
        specularReflected = Speculate(cameraPos, i.worldPos, lv.dirToL, lv.irradiance * specularReflectionColor,
            specularWeight, normal, att, specularPowerLoaded);
        specularReflected += Speculate(cameraPos, i.worldPos, direction, DdiffuseColor * DdiffuseIntensity * specularReflectionColor,
            specularWeight, normal, 1.0f, specularPowerLoaded);
        // scale by shadow level
        diffuse *= shadowLevel;
        specularReflected *= shadowLevel;
    }
    else
    {
        diffuse = specularReflected = 0.0f;
    }
	// final color = attenuate diffuse & ambient by diffuse texture color and add specular reflected
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, i.tc).rgb + specularReflected), 1.0f);
}