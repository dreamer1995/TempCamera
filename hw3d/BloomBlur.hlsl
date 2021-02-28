#include "Constants.hlsli"

Texture2D tex : register(t0);
SamplerState splr;

cbuffer Kernel : register(b10)
{
    uint nTaps;
    float coefficients[15];
}

cbuffer Control : register(b11)
{
    bool horizontal;
}

float4 main(float2 uv : Texcoord) : SV_Target
{
    if (!HDR)
        return 0;
    uint width, height;
    tex.GetDimensions(width, height);
    float dx, dy;
    if (horizontal)
    {
        dx = 1.0f / width;
        dy = 0.0f;
    }
    else
    {
        dx = 0.0f;
        dy = 1.0f / height;
    }
    const int r = nTaps / 2;

    float4 acc = { 0.0f, 0.0f, 0.0f, 0.0f };
    for (int i = -r; i <= r; i++)
    {
        const float2 tc = uv + float2(dx * i, dy * i);
        const float4 s = tex.Sample(splr, tc).rgba;
        const float coef = coefficients[i + r];
        acc += s * coef;
    }
    return acc;
}