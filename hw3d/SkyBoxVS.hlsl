#include "Constants.hlsli"

struct VSOut
{
	float3 tc : Texcoord;
	float4 pos : SV_Position;
};

VSOut main( float3 pos : Position )
{
	VSOut vso;
	vso.tc = pos;
	vso.pos = mul(float4(pos, 0.0f), matrix_MVP).xyww;
	return vso;
}
