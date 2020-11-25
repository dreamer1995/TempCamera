#include "Constants.hlsli"
#include "Algorithms.hlsli"

#define IsPhong

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
    float2 uv : Texcoord;
    float4 shadowHomoPos : ShadowPosition;
};

struct MaterialShadingParameters
{
    uint shadingModelID;
    float3 worldPos;
    float3 baseColor;
    float3 normal;
    float3 specularColor;
    float specularWeight;
    float specularGloss;
};

void GetMaterialParameters(out MaterialShadingParameters matParams, PSIn IN)
{
    matParams.shadingModelID = ShadingModel_Phong;
    matParams.worldPos = IN.worldPos;
    // sample diffuse texture
    matParams.baseColor = DecodeGamma(tex.Sample(splr, IN.uv).xyz);
    // normalize the mesh normal
    matParams.normal = normalize(IN.normal);
    float3 specularReflectionColor;
    float specularPowerLoaded = specularGloss;
    const float4 specularSample = spec.Sample(splr, IN.uv);
    if( useSpecularMap )
    {
        specularReflectionColor = DecodeGamma(specularSample.rgb);
    }
    else
    {
        specularReflectionColor = specularColor;
    }
    if( useGlossAlpha )
    {
        specularPowerLoaded = pow(2.0f, specularSample.a * 13.0f);
    }
    matParams.specularColor = specularReflectionColor;
    matParams.specularWeight = specularWeight;
    matParams.specularGloss = specularPowerLoaded;
}

#include "ForwardRenderingTrunk.hlsli"