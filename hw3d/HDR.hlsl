#include "Constants.hlsli"

Texture2D sceneColor : register(t0);
Texture2D HDRColor : register(t1);
SamplerState splr;

#define EncodeGamma(x) pow(x, 1.0f / 2.2f)
#define EncodeGammaWithAtten(x) pow(x / (x + 1.0f), 1.0f / 2.2f)

float4 main(float2 uv : Texcoord) : SV_Target
{

	float3 color = sceneColor.SampleLevel(splr, uv, 0).rgb;
	
    if (HDR)
    {
        const float A = 2.51f;
        const float B = 0.03f;
        const float C = 2.43f;
        const float D = 0.59f;
        const float E = 0.14f;
        color = (color * (A * color + B)) / (color * (C * color + D) + E);
    }    

	return float4(EncodeGamma(color), 1);
}