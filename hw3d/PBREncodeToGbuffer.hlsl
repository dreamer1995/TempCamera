#include "Constants.hlsli"

#define IsPBR

Texture2D tex;
Texture2D mramap : register(t1);
Texture2D nmap : register(t2);

cbuffer ObjectCBuf : register(b10)
{
	bool enableAbedoMap;
	bool enableMRAMap;
	bool enableNormalMap;
	bool useAbedoMap;
	float3 materialColor;
	bool useMetallicMap;
	bool useRoughnessMap;
	float metallic;
	float roughness;
	bool useNormalMap;
	float normalMapWeight;
};

struct PSIn {
	float3 worldPos : Position;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 binormal : Binormal;
	float2 uv : Texcoord;
};

struct GBufferOutput
{
	float4 GBuffer0 	: SV_Target0;	// GBufferA[R16G16B16A16F]	: Emission.rgb(Obsolete) BaseColor.rgb, Metallic
	float4 GBuffer1 	: SV_Target1;	// GBufferB[R8G8B8A8]		: Roughness, AO, Specular, InShadow(Obsolete), ShadingModelID
	float4 GBuffer2 	: SV_Target2;	// GBufferC[R8G8B8A8]		: N.xyz
	float4 GBuffer3 	: SV_Target3;	// GBufferD[R8G8B8A8]		: CustomData

	float4 GBuffer4 	: SV_Target4;	// GBufferE[R8G8B8A8]		: N.xyz, GILightingLum
	float4 GBuffer5  	: SV_Target5;	// GBufferF[R8G8B8A8]		: BakeNormal.xyz
	float4 GBuffer6 	: SV_Target6;	// GBufferG[R32F]			: Depth
	float4 GBuffer7 	: SV_Target7;	// GBufferG[R32F]			: Depth
//#if USE_PIXEL_OFFSET
//	float  PixelDepth : SV_Depth;     // For pixel offset
//#endif
};

#include "DeferredCommon.hlsli"

GBufferOutput main(PSIn IN)
{
	GBufferOutput OUT;
	
	OUT.GBuffer1.a = EncodeShadingModelID(ShadingModel_PBR);
	//matParams.worldPos = IN.worldPos;
	float3 abedo = materialColor;
	if (enableAbedoMap)
	{
		if (useAbedoMap)
		{
			float4 basecolor = tex.Sample(splr, IN.uv);
#ifdef AlphaTest
			// bail if highly translucent
			clip(basecolor.a < 0.1f ? -1 : 1);
#endif
			abedo *= DecodeGamma(basecolor.rgb);
		}
	}
	OUT.GBuffer0.rgb = abedo;

	float fmetallic = metallic;
	float froughness = roughness;
	float AO = 1.0f;
	if (enableMRAMap)
	{
		float3 MRA = mramap.Sample(splr, IN.uv).rgb;
		if (useMetallicMap)
		{
			fmetallic *= MRA.x;
		}
		if (useRoughnessMap)
		{
			froughness *= MRA.y;
		}
		AO *= MRA.z;
	}
	OUT.GBuffer1.r = froughness;
	OUT.GBuffer0.a = fmetallic;
	OUT.GBuffer1.g = AO;
	OUT.GBuffer1.b = 0.5f;

	float3 normal = normalize(IN.normal);
	if (enableNormalMap)
	{
		if (useNormalMap)
		{
			const float3 mappedNormal = MapNormal(normalize(IN.tangent), normalize(IN.binormal), normal, IN.uv, nmap, splr);
			normal = lerp(normal, mappedNormal, normalMapWeight);
			normal = normalize(normal);
		}
	}
#ifdef AlphaTest
	// flip normal when backface
	if (dot(normal, normalize(cameraPos - IN.worldPos)) <= 0.0f)
	{
		normal = -normal;
	}
#endif
	OUT.GBuffer2.rgb = EncodeNormal(normal);
	return OUT;
}