Texture2D sceneColor : register(t0);
SamplerState splr;

#define EncodeGamma(x) pow(x, 1.0f / 2.2f)
#define EncodeGammaWithAtten(x) pow(x / (x + 1.0f), 1.0f / 2.2f)

float4 main(float2 uv : Texcoord) : SV_Target
{
	float3 color = sceneColor.SampleLevel(splr, uv, 0).rgb;
	
	return float4(EncodeGamma(color), 1);
}