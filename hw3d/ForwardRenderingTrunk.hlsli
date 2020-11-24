#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "ShadingModel.hlsli"

float4 main(PSIn IN) : SV_Target
{
    MaterialShadingParameters matParams;
    GetMaterialParameters(IN, matParams);

#ifdef MASK_BOI
    // bail if highly translucent
    clip(IN.alpha - 0.1f);
#endif

    GBuffer gBuffer;
    DecodeGBuffer(matParams, gBuffer);

    LightData litData;
    EncodeLightData(litData, IN.worldPos);

#ifdef MASK_BOI
    // flip normal when backface
    const float3 viewDir = normalize(cameraPos - i.worldPos);
    if (dot(IN.normal, viewDir) <= 0.0f)
    {
        IN.normal = -IN.normal;
    }
#endif

    const float shadowLevel = Shadow(i.shadowHomoPos);
    if (shadowLevel != 0.0f)
    {
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
    return float4(saturate((diffuse + ambient) * dtex.rgb + specularReflected), 1.0f);
}

void EncodeLightData(out LightData litData, float3 worldPos)
{
    litData.pointIrradiance = diffuseColor * diffuseIntensity;
    float3 vToL = lightPos - worldPos;
    float3 distToL = length(vToL);
    litData.pointDirToL = vToL / distToL;
    litData.atten = Attenuate(attConst, attLin, attQuad, distToL);
    litData.directionalIrradiance = DdiffuseColor * DdiffuseIntensity;
    litData.directionalDirToL = direction;
}