#include "Constants.hlsli"
#include "Algorithms.hlsli"

#define NoTangent
#define NoUV

struct VSIn
{
    float3 pos : Position;
    float3 n : Normal;
};

struct VSOut
{
    float3 worldPos : Position;
    float3 normal : Normal;
    float4 shadowHomoPos : ShadowPosition;
    float4 pos : SV_Position;
};

void GetVertexParameters(inout VSOut o, VSIn v)
{

}

#include "VSTrunk.hlsli"