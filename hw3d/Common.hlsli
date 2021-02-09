#define ShadingModel_UnLit 0u
#define ShadingModel_Phong 1u
#define ShadingModel_PBR 2u
#define ShadingModel_Liquid 3u
#define ShadingModel_Toon 4u

#define DecodeGamma(x) pow(x, 2.2f)
#define EncodeGamma(x) pow(x, 1.0f / 2.2f)
#define EncodeGammaWithAtten(x) pow(x / (x + 1.0f), 1.0f / 2.2f)

Texture2D smap : register(t14);//PS
TextureCube smap0 : register(t15);//PS
TextureCube smap1 : register(t16);//PS
TextureCube smap2 : register(t17);//PS
SamplerComparisonState ssamHw : register(s2);//PS
SamplerState ssamSw : register(s3);//PS

// For Cube Shadowing
static const float zf = 100.0f;
static const float zn = 0.5f;
static const float c1 = zf / (zf - zn);
static const float c0 = -zn * zf / (zf - zn);

float3 MapNormal(
    const in float3 tan,
    const in float3 binor,
    const in float3 normal,
    const in float2 tc,
    uniform Texture2D nmap,
    uniform SamplerState splr)
{
    // build the tranform (rotation) into same space as tan/binor/normal (target space)
	const float3x3 tanToTarget = float3x3(tan, binor, normal);
    // sample and unpack the normal from texture into target space   
	const float3 normalSample = nmap.Sample(splr, tc).xyz;
	const float3 tanNormal = normalSample * 2.0f - 1.0f;
    // bring normal from tanspace into target space
	return normalize(mul(tanNormal, tanToTarget));
}