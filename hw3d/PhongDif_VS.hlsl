#include "Constants.hlsli"
#include "Algorithms.hlsli"

#define NoTangent

struct VSIn
{
	float3 pos : Position;
	float3 n : Normal;
	float2 uv : Texcoord;
};

struct VSOut
{
	float3 worldPos : Position;
	float3 normal : Normal;
	float2 uv : Texcoord;
	float4 shadowHomoPos : ShadowPosition;
	float4 pos : SV_Position;
};

void GetVertexParameters(inout VSOut o, VSIn v)
{

}

#include "VSTrunk.hlsli"