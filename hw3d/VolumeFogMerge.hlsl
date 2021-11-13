StructuredBuffer<float4> sceneColor : register(t0);
SamplerState splr;

float4 main(float2 uv : Texcoord) : SV_Target
{
	float3 color = sceneColor[uv.y * 900 * 1600 + uv.x * 1600].rgb;
	
	return float4(color, 1);
}