#include "Constants.hlsli"

struct VSIn
{
	float3 pos : Position;
	float3 n : Normal;
	float2 tc : Texcoord;
};

struct VSOut
{
	float3 worldPos : Position;
	float3 normal : Normal;
	float2 tc : Texcoord;
	float4 pos : SV_Position;
};

VSOut main(VSIn v)
{
	VSOut o;
	o.pos = mul(float4(v.pos, 1.0f), matrix_MVP);
	o.worldPos = (float3) mul(float4(v.pos, 1.0f), matrix_M2W);
	o.tc = v.tc;
	o.normal = normalize(mul(v.n, (float3x3)matrix_M2W));
	return o;
}