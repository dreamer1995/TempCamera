#define ShadingModel_UnLit 0
#define ShadingModel_Phong 1
#define ShadingModel_PBR 2
#define ShadingModel_Liquid 3
#define ShadingModel_Toon 4

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