#include "ShadingModel.hlsli"

float4 main(PSIn IN) : SV_Target
{
    MaterialShadingParameters matParams;
    GetMaterialParameters(matParams, IN);

    GBuffer gBuffer;
    DecodeGBuffer(matParams, gBuffer);

    LightData litData;
    EncodeLightData(litData, gBuffer.worldPos);

    float3 V = normalize(cameraPos - gBuffer.worldPos);
#ifdef MASK_BOI
    // flip normal when backface
    if (dot(gBuffer.normal, V) <= 0.0f)
    {
        gBuffer.normal = -gBuffer.normal;
    }
#endif
    LightingResult litRes;
    const float shadowLevel = Shadow(IN.shadowHomoPos);
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
    // final color = attenuate diffuse & ambient by diffuse texture color and add specular reflected
    return float4(saturate((litRes.diffuseLighting + ambient) * gBuffer.baseColor + litRes.specularLighting), 1.0f);
}