#include "Constants.hlsli"
#include "Algorithms.hlsli"

#define IsPhong

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
    float4 dtex = tex.Sample(splr, IN.tc);
    matParams.baseColor = dtex.xyz;
    // normalize the mesh normal
    float3 normal = normalize(IN.normal);
    // replace normal with mapped if normal mapping enabled
    if (useNormalMap)
    {
        const float3 mappedNormal = MapNormal(normalize(IN.tan), normalize(IN.binor), normal, IN.tc, nmap, splr);
        normal = lerp(normal, mappedNormal, normalMapWeight);
    }
    matParams.normal = normalize(normal);
    matParams.specularColor = specularColor;
    matParams.specularWeight = specularWeight;
    matParams.specularGloss = specularGloss;
}

#include "ForwardRenderingTrunk.hlsli"