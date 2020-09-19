cbuffer DirectionalLightCBuf : register(b3)//PS
{
    float3 direction;
    float3 DdiffuseColor;
    float DdiffuseIntensity;
};

cbuffer PointLightCBuf : register(b4)//PS
{
    float3 lightPos;
    float3 ambient;
    float3 diffuseColor;
    float diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

struct LightVectorData
{
    float3 irradiance;
    float3 vToL;
    float3 dirToL;
    float distToL;
};

LightVectorData CalculateLightVectorData(const in float3 lightPos, const in float3 fragPos)
{
    LightVectorData lv;
    lv.irradiance = diffuseColor * diffuseIntensity;
    lv.vToL = lightPos - fragPos;
    lv.distToL = length(lv.vToL);
    lv.dirToL = lv.vToL / lv.distToL;
    return lv;
}