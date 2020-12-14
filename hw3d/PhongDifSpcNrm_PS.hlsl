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
    float3 tangent : Tangent;
    float3 binormal : Binormal;
    float2 uv : Texcoord;
    float4 shadowHomoPos : ShadowPosition0;
    float4 shadowCubeWorldPos0 : ShadowPosition1;
    float4 shadowCubeWorldPos1 : ShadowPosition2;
    float4 shadowCubeWorldPos2 : ShadowPosition3;
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
    float4 dtex = tex.Sample(splr, IN.uv);
    matParams.baseColor = DecodeGamma(dtex.xyz);
    // normalize the mesh normal
    float3 normal = normalize(IN.normal);
#ifdef MASK_BOI
    // bail if highly translucent
    clip(dtex.a < 0.1f ? -1 : 1);
    // flip normal when backface
    if (dot(normal, normalize(cameraPos - IN.worldPos)) <= 0.0f)
    {
        normal = -normal;
    }
#endif
    // replace normal with mapped if normal mapping enabled
    if (useNormalMap)
    {
        const float3 mappedNormal = MapNormal(normalize(IN.tangent), normalize(IN.binormal), normal, IN.uv, nmap, splr);
        normal = lerp(normal, mappedNormal, normalMapWeight);
    }
    matParams.normal = normalize(normal);
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