#include "Constants.hlsli"
#include "Algorithms.hlsli"

#define NoShadow

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
	float3 n : Normal;
	float3 t : Tangent;
	float3 b : Binormal;
	float2 uv : Texcoord;
};

void GetVertexParameters(inout VSOut o, VSIn v)
{

}

#include "VSTrunk.hlsli"