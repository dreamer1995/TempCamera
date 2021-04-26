Texture2D volumeColor : register(t0);
Texture2D depth : register(t8);
SamplerState splr;

static const float PI = 3.14159265359;

cbuffer Control : register(b10)
{
	bool horizontal;
}

#include "Constants.hlsli"
#include "Algorithms.hlsli"
#include "DeferredCommon.hlsli"

float4 main(float2 uv : Texcoord) : SV_Target
{
	static const float s_offsets[5] = { -3.5, -1.5, 1.5, 3.5, 0 };
	static const float s_weights[5] = { 1.0 / 16, 1.0 / 4, 1.0 / 4, 1.0 / 16, 3.0 / 8 };
	float c_phi0 = 3.3f;
	float p_phi0 = 5.5f;
	
	float c_phi = 10 * c_phi0;
	float p_phi = p_phi0;
	float stepWidth = 1;

	float4 CurrColor = volumeColor.SampleLevel(splr, uv, 0);
	CurrColor.w = ConvertToLinearDepth(depth.SampleLevel(splr, uv, 0).r);
	
	float SumWeight = s_weights[4];
	float3 SumColor = CurrColor.rgb * s_weights[4];
	
	for (int i = 0; i < 4; ++i)
	{
		float2 tempTC = uv;
		
		if (horizontal)
			tempTC.x += s_offsets[i] * screenInfo.z * stepWidth;
		else
			tempTC.y += s_offsets[i] * screenInfo.w * stepWidth;

		//uv = min(uv, g_pixelSize.zw - 0.5f*g_pixelSize.xy);
		float4 ctmp = volumeColor.SampleLevel(splr, tempTC, 0);
		ctmp.w = ConvertToLinearDepth(depth.SampleLevel(splr, tempTC, 0).r);
		float3 t = CurrColor.rgb - ctmp.rgb;
		float c_dist = dot(t, t);
		//float c_w = exp(-c_dist / c_phi);

		float p_dist = (CurrColor.w - ctmp.w) * (CurrColor.w - ctmp.w);
		//float p_w = exp(-p_dist / p_phi);
		float weight = s_weights[i] * exp(-c_dist / c_phi - p_dist / p_phi); //c_w * n_w * p_w;
		SumColor += ctmp.rgb * weight;
		SumWeight += weight;
	}
	SumColor /= SumWeight;		
	
	return float4(volumeColor.SampleLevel(splr, uv, 0).rgb, 1.0f);
}