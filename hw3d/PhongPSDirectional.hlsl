#include "Constants.hlsli"

#include "LightVectorData.hlsli"
#include "Algorithms.hlsli"

cbuffer ObjectCBuf : register(b10)
{
    float3 color;
    float specularWeight;
    float specularGloss;
};

struct PSIn {
    float3 worldPos : Position;
    float3 normal : Normal;
};

float4 main(PSIn i) : SV_Target
{
    // renormalize interpolated normal
    float3 normal = normalize(i.normal);
    // diffuse
    float3 diffuse = Diffuse(DdiffuseColor * DdiffuseIntensity, 1.0f, direction, normal);
    // specular
    float3 specular = Speculate(cameraPos, i.worldPos, direction, DdiffuseColor * DdiffuseIntensity,
        specularWeight, normal, 1.0f, specularGloss);
    // final color
    return float4(saturate(diffuse * color + specular), 1.0f);
}