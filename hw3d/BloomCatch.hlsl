#include "Constants.hlsli"

Texture2D sceneColor : register(t0);
SamplerState splr;

cbuffer CBufProperties : register(b10)
{
	float BloomThreshold;
};

float Luminance(float3 LinearColor)
{
	return dot(LinearColor, float3(0.3f, 0.59f, 0.11f));
}

float4 main(float2 uv : Texcoord) : SV_Target
{
	if (!HDR)
		return 0;
	float4 SceneColor = sceneColor.SampleLevel(splr, uv, 0);

	// clamp to avoid artifacts from exceeding fp16 through framebuffer blending of multiple very bright lights
	SceneColor.rgb = min(float3(256 * 256, 256 * 256, 256 * 256), SceneColor.rgb);

	half3 LinearColor = SceneColor.rgb;

	half TotalLuminance = Luminance(LinearColor);
	half BloomLuminance = TotalLuminance - BloomThreshold;
	half BloomAmount = saturate(BloomLuminance * 0.5f);

	return float4(BloomAmount * LinearColor, 0);
}