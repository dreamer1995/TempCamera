Texture2D sceneColor : register(t1);
SamplerState splr;

float4 main(float2 uv : Texcoord) : SV_Target
{
	float3 color = sceneColor.SampleLevel(splr, uv, 0).rgb;
	
	return float4(color, 1);
}