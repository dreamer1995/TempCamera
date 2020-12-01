#include "Constants.hlsli"
#include "Algorithms.hlsli"

#define NoTangent
#define NoNormal
#define NoHPos
#define NoShadow

cbuffer ObjectCBuf : register(b10)
{
	float4 A;
	float4 S;
	float4 L;
	float4 w;
	float4 Q;
	float4 Dx;
	float4 Dz;
};

#include "WaveCommon.hlsli"

struct VSOut
{
	float3 worldPos : Position;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 binormal : Binormal;
	float2 uv : Texcoord;
	float4 pos : SV_Position;
};

struct VSIn {
	float3 pos : Position;
	float3 t : Tangent;
	float3 b : Binormal;
	float3 n : Normal;
	float2 uv : Texcoord;
};

void GetVertexParameters(inout VSOut o, VSIn v)
{
	const float4 phase = CalculatePhase(o.worldPos);
	float4 sinp, cosp;
	sincos(phase, sinp, cosp);

	float3 disPos = CalculateWavesDisplacement(o.worldPos, sinp, cosp);

	//float depthmap = hmap.SampleLevel(splr, v.uv, 0.0f).r;
	float depthmap = (o.worldPos.z * 0.1f + 1.0f) * 0.5f;

	o.worldPos = o.worldPos + float3(disPos.x, disPos.y * depthmap, disPos.z);

	o.tangent = normalize(CalculateTangent(sinp, cosp));
	o.binormal = -normalize(CalculateBinormal(sinp, cosp));

	o.tangent = normalize(lerp(normalize(mul(v.t, (float3x3)matrix_M2W)), o.tangent, depthmap));
	o.binormal = normalize(lerp(normalize(mul(v.b, (float3x3)matrix_M2W)), o.binormal, depthmap));
	o.normal = normalize(cross(o.tangent, o.binormal));

	o.pos = float4(v.pos.xy * 0.2f, 0.0f, 1.0f);
}

#include "VSTrunk.hlsli"
