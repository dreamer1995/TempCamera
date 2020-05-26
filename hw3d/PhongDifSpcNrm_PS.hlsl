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
    bool useNormalMap;
    float normalMapWeight;
};

Texture2D tex;
Texture2D spec;
Texture2D nmap;

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
    // sample diffuse texture
    float4 dtex = tex.Sample(splr, i.tc);

    #ifdef MASK_BOI
    // bail if highly translucent
    clip(dtex.a < 0.1f ? -1 : 1);
    // flip normal when backface
    const float3 viewDir = normalize(cameraPos - i.worldPos);
    if (dot(normal, viewDir) <= 0.0f)
    {
        normal = -normal;
    }
    #endif

    // replace normal with mapped if normal mapping enabled
    if (useNormalMap)
    {
        const float3 mappedNormal = MapNormal(normalize(i.tan), normalize(i.binor), normal, i.tc, nmap, splr);
        normal = lerp(normal, mappedNormal, normalMapWeight);
    }
	// fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(lightPos, i.worldPos);
    // specular parameter determination (mapped or uniform)
    float3 specularReflectionColor;
    float specularPowerLoaded = specularGloss;
    const float4 specularSample = spec.Sample(splr, i.tc);
    if( useSpecularMap )
    {
        specularReflectionColor = specularSample.rgb;
    }
    else
    {
        specularReflectionColor = specularColor;
    }
    if( useGlossAlpha )
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
    return float4(saturate((diffuse + ambient) * dtex.rgb + specularReflected), 1.0f);
}