#include "Constants.hlsli"
#include "DeferredCommon.hlsli"

Texture2D sceneColor : register(t0);
Texture2D historyColor : register(t1);
Texture2D scenedepth : register(t8);
SamplerState splr;

cbuffer ShadowTransformCBuf0 : register(b10)
{
	matrix matrix_MVP_Last;
};

struct FCatmullRomSamples
{
	// Constant number of samples (BICUBIC_CATMULL_ROM_SAMPLES)
	uint Count;

	// Constant sign of the UV direction from master UV sampling location.
	int2 UVDir[5];

	// Bilinear sampling UV coordinates of the samples
	float2 UV[5];

	// Weights of the samples
	float Weight[5];

	// Final multiplier (it is faster to multiply 3 RGB values than reweights the 5 weights)
	float FinalMultiplier;
};

void Bicubic2DCatmullRom(in float2 UV, in float2 Size, in float2 InvSize, out float2 Sample[3], out float2 Weight[3])
{
	UV *= Size;

	float2 tc = floor(UV - 0.5) + 0.5;
	float2 f = UV - tc;
	float2 f2 = f * f;
	float2 f3 = f2 * f;

	float2 w0 = f2 - 0.5 * (f3 + f);
	float2 w1 = 1.5 * f3 - 2.5 * f2 + 1;
	float2 w3 = 0.5 * (f3 - f2);
	float2 w2 = 1 - w0 - w1 - w3;

	Weight[0] = w0;
	Weight[1] = w1 + w2;
	Weight[2] = w3;

	Sample[0] = tc - 1;
	Sample[1] = tc + w2 / Weight[1];
	Sample[2] = tc + 2;

	Sample[0] *= InvSize;
	Sample[1] *= InvSize;
	Sample[2] *= InvSize;
}

FCatmullRomSamples GetBicubic2DCatmullRomSamples(float2 UV, float2 Size, in float2 InvSize)
{
	FCatmullRomSamples Samples;
	Samples.Count = 5;

	float2 Weight[3];
	float2 Sample[3];
	Bicubic2DCatmullRom(UV, Size, InvSize, Sample, Weight);

	// Optimized by removing corner samples
	Samples.UV[0] = float2(Sample[1].x, Sample[0].y);
	Samples.UV[1] = float2(Sample[0].x, Sample[1].y);
	Samples.UV[2] = float2(Sample[1].x, Sample[1].y);
	Samples.UV[3] = float2(Sample[2].x, Sample[1].y);
	Samples.UV[4] = float2(Sample[1].x, Sample[2].y);

	Samples.Weight[0] = Weight[1].x * Weight[0].y;
	Samples.Weight[1] = Weight[0].x * Weight[1].y;
	Samples.Weight[2] = Weight[1].x * Weight[1].y;
	Samples.Weight[3] = Weight[2].x * Weight[1].y;
	Samples.Weight[4] = Weight[1].x * Weight[2].y;

	Samples.UVDir[0] = int2(0, -1);
	Samples.UVDir[1] = int2(-1, 0);
	Samples.UVDir[2] = int2(0, 0);
	Samples.UVDir[3] = int2(1, 0);
	Samples.UVDir[4] = int2(0, 1);

	// Reweight after removing the corners
	float CornerWeights;
	CornerWeights = Samples.Weight[0];
	CornerWeights += Samples.Weight[1];
	CornerWeights += Samples.Weight[2];
	CornerWeights += Samples.Weight[3];
	CornerWeights += Samples.Weight[4];
	Samples.FinalMultiplier = 1 / CornerWeights;

	return Samples;
}

float4 main(float2 uv : Texcoord) : SV_Target
{
	if (!TAA)
		return float4(sceneColor.SampleLevel(splr, uv, 0));
	
	float4 OUT = 0;
	
	static const int2 kOffsets3x3[9] = { int2(-1, -1), int2(0, -1), int2(1, -1), int2(-1, 0), int2(0, 0), int2(1, 0), int2(-1, 1), int2(0, 1), int2(1, 1) };
	static const float sampleWeights[9] = { 0.01, 0.1, 0.01, 0.1, 1, 0.1, 0.01, 0.1, 0.01 };
	
	float2 screenPos = ScreenSpace2NDC(uv);
	
	float3 posN = 0;
	posN.xy = screenPos;
	float depth = ConvertToLinearDepth(scenedepth.SampleLevel(splr, uv, 0).r);
	posN.z = depth;
	
	float2 velocityOffset = 0;
	{
		float4 depths;
		depths.x = ConvertToLinearDepth(scenedepth.SampleLevel(splr, uv, 0, int2(-2, -2)));
		depths.y = ConvertToLinearDepth(scenedepth.SampleLevel(splr, uv, 0, int2(2, -2)));
		depths.z = ConvertToLinearDepth(scenedepth.SampleLevel(splr, uv, 0, int2(-2, 2)));
		depths.w = ConvertToLinearDepth(scenedepth.SampleLevel(splr, uv, 0, int2(2, 2)));
		
		float2 DepthOffset = float2(2, 2);
		float DepthOffsetXx = 2;

		if (depths.x > depths.y)
		{
			DepthOffsetXx = -2;
		}
		if (depths.z > depths.w)
		{
			DepthOffset.x = -2;
		}
		float depthsXY = max(depths.x, depths.y);
		float depthsZW = max(depths.z, depths.w);
		if (depthsXY > depthsZW)
		{
			DepthOffset.y = -2;
			DepthOffset.x = DepthOffsetXx;
		}
		float depthsXYZW = max(depthsXY, depthsZW);
		if (depthsXYZW > posN.z)
		{
			velocityOffset = DepthOffset * screenInfo.zw;
			posN.z = depthsXYZW;
		}
	}
	
	bool bOffscreen = false;
	float velocity = 0;
	float historyBlur = 0;
	float2 historyScreenPosition = screenPos;

	float3 worldPos = CalcHomogeneousPos(depth, uv);
	worldPos += cameraPos.xyz;
	float4 lastPos = mul(float4(worldPos, 1.0f), matrix_MVP_Last);
	float2 lastUV = lastPos.xy / lastPos.w;
	lastUV = NDC2ScreenSpace(lastUV);
	//lastUV += stc - tc;
	float2 lastScreenPos = ScreenSpace2NDC(lastUV);
	
	float2 backN = lastUV - uv;
	float2 backT = backN * screenInfo.xy;
	
	velocity = sqrt(dot(backT, backT));
	//Test = velocity / 40;
	historyScreenPosition = uv + backN;
	bOffscreen = max(historyScreenPosition.x, historyScreenPosition.y) > 1.0f || min(historyScreenPosition.x, historyScreenPosition.y) < 0.0f;
	
	float4 allNeighborSceneColor[9];
	for (uint j = 0; j < 9; j++)
	{
		allNeighborSceneColor[j].rgb = RGB2YCoCg(sceneColor.SampleLevel(splr, uv, 0, kOffsets3x3[j]).rgb);
		allNeighborSceneColor[j].a = 0;
	}
	
	float4 filtered = 0;
	{
		float neighborsHDRWeight = 0;
		float neighborsFinalWeight = 0;
		float4 neighborsColor = 0;

		for (uint k = 0; k < 9; k++)
		{
			float hdrWeight = rcp(allNeighborSceneColor[k].x + 4.0);
			float spatialWeight = sampleWeights[k];
			float sampleFinalWeight = hdrWeight * spatialWeight;
			neighborsColor.rgb += sampleFinalWeight * allNeighborSceneColor[k].rgb;
			neighborsFinalWeight += sampleFinalWeight;
			neighborsHDRWeight += sampleFinalWeight;
		}

		filtered.rgb = neighborsColor.rgb * rcp(neighborsFinalWeight);
		filtered.a = 0;

		float2 testPos = abs(uv * 2.0f - 1.0f) + screenInfo.zw * 1.0;
			
		if (max(testPos.x, testPos.y) >= 1.0)
		{
			filtered.rgb = allNeighborSceneColor[4];
		}
	}
	
	float4 neighborMin = 0;
	float4 neighborMax = 0;
	neighborMin.rgb = Min3x3(allNeighborSceneColor[1].rgb, allNeighborSceneColor[3].rgb, allNeighborSceneColor[4].rgb);;
	neighborMin.rgb = Min3x3(neighborMin.rgb, allNeighborSceneColor[5].rgb, allNeighborSceneColor[7].rgb);
	neighborMax.rgb = Max3x3(allNeighborSceneColor[1].rgb, allNeighborSceneColor[3].rgb, allNeighborSceneColor[4].rgb);
	neighborMax.rgb = Max3x3(neighborMax.rgb, allNeighborSceneColor[5].rgb, allNeighborSceneColor[7].rgb);
	float4 neighborMinPlus = neighborMin;
	float4 neighborMaxPlus = neighborMax;
	neighborMin.rgb = Min3x3(neighborMin.rgb, allNeighborSceneColor[0].rgb, allNeighborSceneColor[2].rgb);
	neighborMin.rgb = Min3x3(neighborMin.rgb, allNeighborSceneColor[6].rgb, allNeighborSceneColor[8].rgb);
	neighborMax.rgb = Max3x3(neighborMax.rgb, allNeighborSceneColor[0].rgb, allNeighborSceneColor[2].rgb);
	neighborMax.rgb = Max3x3(neighborMax.rgb, allNeighborSceneColor[6].rgb, allNeighborSceneColor[8].rgb);
	
	neighborMin.rgb = neighborMin.rgb * 0.5 + neighborMinPlus.rgb * 0.5;
	neighborMax.rgb = neighborMax.rgb * 0.5 + neighborMaxPlus.rgb * 0.5;
	
	float4 history = 0;
	float2 historyUV = NDC2ScreenSpace(historyScreenPosition);
	historyUV = historyScreenPosition;
	FCatmullRomSamples Samples = GetBicubic2DCatmullRomSamples(historyUV, screenInfo.xy, screenInfo.zw);
	for (uint i = 0; i < 5; i++)
	{
		float2 SampleUV = Samples.UV[i];

		if (Samples.UVDir[i].x < 0)
		{
			SampleUV.x = max(SampleUV.x, 0);
		}
		else if (Samples.UVDir[i].x > 0)
		{
			SampleUV.x = min(SampleUV.x, 1);
		}

		if (Samples.UVDir[i].y < 0)
		{
			SampleUV.y = max(SampleUV.y, 0);
		}
		else if (Samples.UVDir[i].y > 0)
		{
			SampleUV.y = min(SampleUV.y, 1);
		}
		history += historyColor.SampleLevel(splr, SampleUV, 0) * Samples.Weight[i];
	}
	history *= Samples.FinalMultiplier;

	//history = tLastScreenMap.SampleLevel(sLastScreenSample, historyUV, 0);
	history.rgb = RGB2YCoCg(history.rgb);
	
	//Anti-Ghost

	float lumaMin = neighborMin.x;
	float lumaMax = neighborMax.x;
	float lumaHistory = history.x;

	float4 preClampingHistory = history;
	history.rgb = clamp(history.rgb, neighborMin.rgb, neighborMax.rgb);

	float blendFinal = 0.04;
	{
		float lumaFiltered = filtered.x;

		blendFinal = lerp(blendFinal, 0.2, saturate(velocity / 40));

		// Anti-flicker
		float distToClamp = 2 * abs(min(lumaHistory - lumaMin, lumaMax - lumaHistory) / (lumaMax - lumaMin));
		blendFinal *= lerp(0, 1, saturate(4 * distToClamp));
		blendFinal += 0.8 * saturate(0.02 * lumaHistory / abs(filtered.x - lumaHistory));
		blendFinal *= (lumaMin + 0.5) / (lumaMax + 0.5);

		blendFinal = max(blendFinal, saturate(0.01 * lumaHistory / abs(lumaFiltered - lumaHistory)));
	}

	if (bOffscreen)
	{
		history = filtered;
	}
	
	float filteredWeight = rcp(filtered.x + 4.0f);
	float historyWeight = rcp(history.x + 4.0f);
	float2 finalWeight = WeightedLerpFactors(historyWeight, filteredWeight, blendFinal);

	float3 oc = history.rgb * finalWeight.x + filtered.rgb * finalWeight.y;

	OUT.rgb = YCoCg2RGB(oc);
	
	return OUT;
}