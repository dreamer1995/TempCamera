#include "ShadingModel.hlsli"
#include "ConstantsPS.hlsli"

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
    float shadowLevel = 1.0f;

    if (lightCount > 0u)
    {
        //shadowLevel = CubeShadow(IN.shadowCubeWorldPos0, smap0);
        shadowLevel = 1.0f;
        if (shadowLevel != 0.0f)
        {
            EncodePLightData(litData, diffuseColor * diffuseIntensity, lightPos - gBuffer.worldPos, attConst, attLin, attQuad);
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
    }
    
    if (lightCount > 1u)
    {
        shadowLevel = CubeShadow(IN.shadowCubeWorldPos1, smap1);
        if (shadowLevel != 0.0f)
        {
            EncodePLightData(litData, diffuseColor2 * diffuseIntensity2, lightPos2 - gBuffer.worldPos, attConst2, attLin2, attQuad2);
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
    }
    
    if (lightCount > 2u)
    {
        shadowLevel = CubeShadow(IN.shadowCubeWorldPos2, smap2);
        if (shadowLevel != 0.0f)
        {
            EncodePLightData(litData, diffuseColor3 * diffuseIntensity3, lightPos3 - gBuffer.worldPos, attConst3, attLin3, attQuad3);
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
    }

    //shadowLevel = Shadow(IN.shadowHomoPos, smap);
    shadowLevel = 1.0f;
    if (shadowLevel != 0.0f)
    {
        EncodeDLightData(litData, DdiffuseColor * DdiffuseIntensity, direction);
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
    BxDF_Ambient(ambientLighting, gBuffer, V, ambient);

    // final color = attenuate diffuse & ambient by diffuse texture color and add specular reflected
    outCol = diffuseLighting + specularLighting + ambientLighting;

    outCol = EncodeGamma(outCol);

    return float4(outCol, 1.0f);
}