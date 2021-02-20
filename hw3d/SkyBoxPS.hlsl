#include "Common.hlsli"

cbuffer CBuf : register(b10)
{
	float4 materialColor;
};

TextureCube SkyMap;
SamplerState splr;

float4 main(float3 tc : Texcoord) : SV_Target
{
	float3 col = DecodeGamma(SkyMap.Sample(splr, normalize(tc))) * materialColor;
	
	return float4(col, 1.0f);
}

//RasterizerState MyCull {
//	FillMode = WireFrame;
//};
//
//technique11 main11
//{
//	pass p0
//	{
//		SetRasterizerState(MyCull);
//	}
//};