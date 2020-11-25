cbuffer TransformCBuf : register(b0)//VS, PS
{
	matrix matrix_MVP;
	matrix matrix_MV;
	matrix matrix_V;
	matrix matrix_P;
	matrix matrix_VP;
	matrix matrix_T_MV;
	matrix matrix_IT_MV;
	matrix matrix_M2W;
	matrix matrix_W2M;
};

cbuffer CameraCBuf : register(b1)//VS, PS
{
	float3 cameraPos;
	float3 cameraDir;
};

cbuffer Time : register(b2)//VS, DS, PS
{
	float time;
	matrix EVRotation;
};

cbuffer DirectionalLightCBuf : register(b3)//PS
{
	float3 direction;
	float3 DdiffuseColor;
	float DdiffuseIntensity;
};

cbuffer PointLightCBuf : register(b4)//PS
{
	float3 lightPos;
	float3 ambient;
	float3 diffuseColor;
	float diffuseIntensity;
	float attConst;
	float attLin;
	float attQuad;
};

cbuffer ShadowTransformCBuf : register(b5)//VS
{
	matrix shadowMatrix_VP;
};

cbuffer ShadowControl : register(b6)//PS
{
	int pcfLevel;
	float depthBias;
	bool hwPcf;
}

#define ShadingModel_UnLit 0
#define ShadingModel_Phong 1
#define ShadingModel_PBR 2
#define ShadingModel_Liquid 3
#define ShadingModel_Toon 4

#define DecodeGamma(x) pow(x, 2.2f)
#define EncodeGamma(x) pow(x / (x + 1.0f), 1.0f / 2.2f)

Texture2D smap : register(t14);//PS
SamplerComparisonState ssamHw : register(s2);//PS
SamplerState ssamSw : register(s3);//PS
