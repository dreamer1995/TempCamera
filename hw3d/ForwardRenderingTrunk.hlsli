#include "ShadingModel.hlsli"

float4 main(PSIn IN) : SV_Target
{
    float3 outCol = 0;
    MaterialShadingParameters matParams;
    GetMaterialParameters(matParams, IN);

    GBuffer gBuffer;
    DecodeGBuffer(matParams, gBuffer);

    float3 V = normalize(cameraPos - gBuffer.worldPos);
#ifdef MASK_BOI
    // flip normal when backface
    if (dot(gBuffer.normal, V) <= 0.0f)
    {
        gBuffer.normal = -gBuffer.normal;
    }
#endif

    // Dynamic Lighting
    float3 diffuseLighting = 0.0f;
    float3 specularLighting = 0.0f;

    LightingResult litRes;
    LightData litData;
    EncodeLightData(litData, diffuseColor * diffuseIntensity, lightPos - gBuffer.worldPos, false);
    float shadowLevel = 1.0f;
    if (shadowLevel != 0.0f)
    {
        BxDF(litRes, gBuffer, litData, V, shadowLevel);
        // scale by shadow level
        litRes.diffuseLighting *= shadowLevel;
        litRes.specularLighting *= shadowLevel;
    }
    else
    {
        litRes.diffuseLighting = litRes.specularLighting = 0.0f;
    }
    diffuseLighting += litRes.diffuseLighting;
    specularLighting += litRes.specularLighting;

    EncodeLightData(litData, DdiffuseColor * DdiffuseIntensity, direction, true);
    shadowLevel = Shadow(IN.shadowHomoPos);
    if (shadowLevel != 0.0f)
    {
        BxDF(litRes, gBuffer, litData, V, shadowLevel);
        // scale by shadow level
        litRes.diffuseLighting *= shadowLevel;
        litRes.specularLighting *= shadowLevel;
    }
    else
    {
        litRes.diffuseLighting = litRes.specularLighting = 0.0f;
    }
    diffuseLighting += litRes.diffuseLighting;
    specularLighting += litRes.specularLighting;

    // Ambient Lighting
    float3 ambientLighting;
    BxDF_Ambient(ambientLighting, gBuffer, V);

    // final color = attenuate diffuse & ambient by diffuse texture color and add specular reflected
    outCol = diffuseLighting + specularLighting + ambientLighting;

    outCol = EncodeGammaWithAtten(outCol);

    return float4(outCol, 1.0f);
}