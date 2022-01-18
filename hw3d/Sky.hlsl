Texture2D transmittanceLUT : register(t0);
Texture2D scatteringLUT : register(t1);
Texture2D SkyViewLutTexture : register(t2);
Texture3D AtmosphereCameraScatteringVolume : register(t3);
Texture2D depth : register(t8);

SamplerState splr;

#define MULTISCATAPPROX_ENABLED 1
#define FASTSKY_ENABLED 1
#define RENDER_SUN_DISK 1
#define SHADOWMAP_ENABLED 1 

#include "Constants.hlsli"
#include "SkyAtmosphereCommon.hlsli"

struct RayMarchPixelOutputStruct
{
	float4 Luminance : SV_TARGET0;
#if COLORED_TRANSMITTANCE_ENABLED
	float4 Transmittance	: SV_TARGET1;
#endif
};

RayMarchPixelOutputStruct main(float2 uv : Texcood)
{
	RayMarchPixelOutputStruct output = (RayMarchPixelOutputStruct) 0;
#if COLORED_TRANSMITTANCE_ENABLED
	output.Transmittance = float4(0, 0, 0, 1);
#endif

	float2 pixPos = uv * screenInfo.xy;
	AtmosphereParameters Atmosphere = GetAtmosphereParameters();

	float3 ClipSpace = float3(uv * float2(2.0f, -2.0f) - float2(1.0f, -1.0f), 1.0f);
	float4 HPos = mul(float4(ClipSpace, 1.0f), matrix_I_VP);
	HPos.xyz = HPos.xzy;
	
	float3 WorldDir = normalize(HPos.xyz / HPos.w - cameraPos.xzy);
	float3 WorldPos = cameraPos.xzy + float3(0, 0, Atmosphere.BottomRadius);

	float DepthBufferValue = -1.0f;


	//if (pixPos.x < 512 && pixPos.y < 512)
	//{
	//	output.Luminance = float4(MultiScatTexture.SampleLevel(samplerLinearClamp, pixPos / float2(512, 512), 0).rgb, 1.0);
	//	return output;
	//}


	float viewHeight = length(WorldPos);
	float3 L = 0;
	DepthBufferValue = depth[pixPos].r;
#if FASTSKY_ENABLED
	if (viewHeight < Atmosphere.TopRadius && DepthBufferValue == 1.0f)
	{
		float2 uv;
		float3 UpVector = normalize(WorldPos);
		float viewZenithCosAngle = dot(WorldDir, UpVector);

		float3 sideVector = normalize(cross(UpVector, WorldDir));		// assumes non parallel vectors
		float3 forwardVector = normalize(cross(sideVector, UpVector));	// aligns toward the sun light but perpendicular to up vector
		float2 lightOnPlane = float2(dot(direction.xzy, forwardVector), dot(direction.xzy, sideVector));
		lightOnPlane = normalize(lightOnPlane);
		float lightViewCosAngle = lightOnPlane.x;

		bool IntersectGround = raySphereIntersectNearest(WorldPos, WorldDir, float3(0, 0, 0), Atmosphere.BottomRadius) >= 0.0f;

		SkyViewLutParamsToUv(Atmosphere, IntersectGround, viewZenithCosAngle, lightViewCosAngle, viewHeight, uv);


		//output.Luminance = float4(SkyViewLutTexture.SampleLevel(samplerLinearClamp, pixPos / float2(gResolution), 0).rgb + GetSunLuminance(WorldPos, WorldDir, Atmosphere.BottomRadius), 1.0);
		output.Luminance = float4(SkyViewLutTexture.SampleLevel(splr, uv, 0).rgb + GetSunLuminance(WorldPos, WorldDir, Atmosphere.BottomRadius, direction.xzy), 1.0f);
		return output;
	}
#else
	if (DepthBufferValue == 1.0f)
		L += GetSunLuminance(WorldPos, WorldDir, Atmosphere.BottomRadius);
#endif

#if FASTAERIALPERSPECTIVE_ENABLED

#if COLORED_TRANSMITTANCE_ENABLED
#error The FASTAERIALPERSPECTIVE_ENABLED path does not support COLORED_TRANSMITTANCE_ENABLED.
#else

	ClipSpace = float3(uv * float2(2.0f, -2.0f) - float2(1.0f, -1.0f), DepthBufferValue);
	float4 DepthBufferWorldPos = mul(float4(ClipSpace, 1.0f), matrix_I_VP);
	DepthBufferWorldPos.xyz = DepthBufferWorldPos.xzy;
	DepthBufferWorldPos /= DepthBufferWorldPos.w;
	float tDepth = length(DepthBufferWorldPos.xyz - (WorldPos + float3(0.0, 0.0, -Atmosphere.BottomRadius)));
	float Slice = AerialPerspectiveDepthToSlice(tDepth);
	float Weight = 1.0f;
	if (Slice < 0.5f)
	{
		// We multiply by weight to fade to 0 at depth 0. That works for luminance and opacity.
		Weight = saturate(Slice * 2.0f);
		Slice = 0.5f;
	}
	float w = sqrt(Slice / AP_SLICE_COUNT);	// squared distribution

	const float4 AP = Weight * AtmosphereCameraScatteringVolume.SampleLevel(splr, float3(uv, w), 0);
	L.rgb += AP.rgb;
	float Opacity = AP.a;

	output.Luminance = float4(L, Opacity);
	//output.Luminance *= frac(clamp(w*AP_SLICE_COUNT, 0, AP_SLICE_COUNT));
#endif

#else // FASTAERIALPERSPECTIVE_ENABLED

	// Move to top atmosphere as the starting point for ray marching.
	// This is critical to be after the above to not disrupt above atmosphere tests and voxel selection.
	if (!MoveToTopAtmosphere(WorldPos, WorldDir, Atmosphere.TopRadius))
	{
		// Ray is not intersecting the atmosphere		
		output.Luminance = float4(GetSunLuminance(WorldPos, WorldDir, Atmosphere.BottomRadius, direction.xzy), 1.0f);
		return output;
	}

	const bool ground = false;
	const float SampleCountIni = 0.0f;
	const bool VariableSampleCount = true;
	const bool MieRayPhase = true;
	SingleScatteringResult ss = IntegrateScatteredLuminance(pixPos, WorldPos, WorldDir, direction.xzy, Atmosphere, ground, SampleCountIni, DepthBufferValue, VariableSampleCount, MieRayPhase);

	L += ss.L;
	float3 throughput = ss.Transmittance;

#if COLORED_TRANSMITTANCE_ENABLED
	output.Luminance = float4(L, 1.0f);
	output.Transmittance = float4(throughput, 1.0f);
#else
	const float Transmittance = dot(throughput, float3(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f));
	output.Luminance = float4(L, 1.0 - Transmittance);
#endif

#endif // FASTAERIALPERSPECTIVE_ENABLED

	return output;
}