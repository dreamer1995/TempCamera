#include <PBRHeader.hlsli>
#include "Constants.hlsli"

#include "LightVectorData.hlsli"
#include "Algorithms.hlsli"

Texture2D tex;
Texture2D nmap : register(t1);
Texture2D rmamap : register(t2);

cbuffer ObjectCBuf : register(b4)
{
	float3 baseColor;
	float roughness;
	float metallic;
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

float4 main(PSIn i) : SV_Target
{
	// normalize the mesh normal
	float3 normal = normalize(i.normal);
	// replace normal with mapped if normal mapping enabled
	if (useNormalMap)
	{
		const float3 mappedNormal = MapNormal(normalize(i.tangent), normalize(i.binormal), normal, i.uv, nmap, splr);
		normal = lerp(normal, mappedNormal, normalMapWeight);
	}

	//const float3 PlightDir = normalize(lightPos - i.worldPos);

	//const float distToL = length(lightPos - i.worldPos);

	const float3 viewDir = normalize(cameraPos - i.worldPos);
	//PBR Start
	const float NdotV = max(dot(normal, viewDir), 0.0f);
	const float NdotL = max(dot(normal, direction), 0.0f);
	const float3 halfDir = normalize(direction + viewDir);
	float NdotH = max(dot(normal, halfDir), 0.0f);
	//float3 rotatedNormal = normalize(mul(i.normal, (float3x3)EVRotation));
	float3 R = reflect(-viewDir, normal);
	//float3 albedo = tex.Sample(splr, i.uv).rgb * color;
	//float3(1.0f, 0.0f, 0.0f)
	
	float3 albedo = pow(tex.Sample(splr, i.uv).rgb, 2.2f) * baseColor;

	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	float3 fRMAmap = rmamap.Sample(splr, i.uv).rgb;
	float fMetallic = metallic * fRMAmap.r;
	F0 = lerp(F0, albedo, fMetallic);
	//fixed3 ambient = UNITY_LIGHTMODEL_AMBIENT.xyz * albedo;

	//const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));

	const float3 radiance = DdiffuseColor * DdiffuseIntensity;
	float fRoughness = roughness * fRMAmap.g;

	float NDF = DistributionGGX(NdotH, fRoughness);
	float G = GeometrySmith(NdotV, NdotL, fRoughness);
	float3 F = FresnelSchlick(max(dot(halfDir, viewDir), 0.0f), F0);

	float3 kS = F;
	float3 kD = 1.0f - kS;
	kD *= 1.0f - fMetallic;

	float3 numerator = NDF * G * F;
	float denominator = 4.0f * NdotV * NdotL + 0.001f;
	float3 specular = numerator / denominator;

	float3 Light = (kD * albedo / PI + specular) * radiance * NdotL;

	float3 iKS = FresnelSchlickRoughness(NdotV, F0, fRoughness);
	float3 iKD = 1.0 - iKS;
	iKD *= 1.0 - fMetallic;
	float3 irradiance = pow(SkyMap.Sample(splr, normal).rgb, 2.2f);
	float3 iDiffuse = irradiance * albedo;

	const float MAX_REF_LOD = 4.0f;
	float3 prefilteredColor = pow(SkyMapMip.SampleLevel(splr, R, fRoughness * MAX_REF_LOD).rgb, 2.2f);
	float2 brdf = BRDFLUT.Sample(splrClamp, float2(NdotV, fRoughness)).rg;
	float3 iSpecular = prefilteredColor * (iKS * brdf.x + brdf.y);


	float ao = fRMAmap.b;
	float3 ambient = (iKD * iDiffuse + iSpecular) * ao;

	float3 color = Light + ambient;

	color = color / (color + 1.0f);
	color = pow(color, 1.0f / 2.2f);



	//SkyMapMip.SampleLevel(splr, i.worldPos, roughness * MAX_REF_LOD).rgb
	return float4(color, 1.0f);
	//const float3 diffuse = PdiffuseColor * PdiffuseIntensity * att * max(0, dot(i.normal, PlightDir)) +
	//						DdiffuseColor * DdiffuseIntens6ity * max(0, dot(i.normal, direction));

	//const float3 PhalfDir = normalize(PlightDir + viewDir);

	//const float3 specular = PdiffuseColor * PdiffuseIntensity * att * specularIntensity * pow(max(0, dot(i.normal, PhalfDir)), specularPower) +
	//						DdiffuseColor * DdiffuseIntensity* specularIntensity * pow(max(0, dot(i.normal, DhalfDir)), specularPower);

	//return float4(ambient + diffuse * albedo + specular, 1.0);
}