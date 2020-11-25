#include "Constants.hlsli"
#include "Algorithms.hlsli"

#define IsPhong

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
    matParams.baseColor = tex.Sample(splr, IN.tc).xyz;
    // normalize the mesh normal
    matParams.normal = normalize(IN.normal);
    matParams.specularColor = specularColor;
    matParams.specularWeight = specularWeight;
    matParams.specularGloss = specularGloss;
}

#include "ForwardRenderingTrunk.hlsli"