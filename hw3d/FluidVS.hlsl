#include "Constants.hlsli"
#include "Algorithms.hlsli"

#define NoTangent
#define NoHPos
#define NoNormal

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

cbuffer ObjectCBuf2 : register(b11)
{
	float3 color;
	float3 attenuation;
	float3 scatteringKd;
	float depth;
};

#include "WaveCommon.hlsli"

struct VSOut
{
	float3 worldPos : Position;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 binormal : Binormal;
	float2 uv : Texcoord;
	float3 outScattering : Position1;
	float3 inScattering : Position2;
	float4 shadowHomoPos : ShadowPosition;
	float4 pos : SV_Position;
};

Texture2D hmap : register(t4);
SamplerState splr;

struct VSIn {
	float3 pos : Position;
	float3 n : Normal;
	float3 t : Tangent;
	float3 b : Binormal;
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

	o.pos = mul(float4(o.worldPos, 1.0f), matrix_VP);

	o.tangent = normalize(CalculateTangent(sinp, cosp));
	o.binormal = -normalize(CalculateBinormal(sinp, cosp));

	o.tangent = normalize(lerp(normalize(mul(v.t, (float3x3)matrix_M2W)), o.tangent, depthmap));
	o.binormal = normalize(lerp(normalize(mul(v.b, (float3x3)matrix_M2W)), o.binormal, depthmap));
	o.normal = normalize(cross(o.tangent, o.binormal));

	const float3 viewDir = normalize(cameraPos - o.worldPos);

	float depthR = max(0, depth + disPos.y) * 2.0f;

	float t = depthR * depthmap / (cameraPos.y - o.worldPos.y);

	//Water scattering

	float d = length(cameraPos - o.worldPos) * t;  // one way!

	o.outScattering = exp(-attenuation * d);

	o.inScattering = color * (1 - o.outScattering * exp(-depthR * depthmap * scatteringKd));
}

#include "VSTrunk.hlsli"
