cbuffer CameraCBuf : register(b1) //VS, PS
{
	float3 cameraPos;
	float3 cameraDir;
	float2 cameraFNPlane;
	float4 vWBasisX;
	float4 vWBasisY;
	float4 vWBasisZ;
};

Texture2D gbuffer[8] : register(t0);
Texture2D depth : register(t8);
SamplerState splr;

float ConvertToLinearDepth(float depth)
{
	// Calculate our projection constants (you should of course do this in the app code, I'm just showing how to do it)
	float ProjectionA = cameraFNPlane.x / (cameraFNPlane.x - cameraFNPlane.y);
	float ProjectionB = (-cameraFNPlane.x * cameraFNPlane.y) / (cameraFNPlane.x - cameraFNPlane.y);

	// Sample the depth and convert to linear view space Z (assume it gets sampled as
	// a floating point value of the range [0,1])
	float linearDepth = ProjectionB / (depth - ProjectionA) / cameraFNPlane.x;
	return linearDepth;
	//return CameraInfo.x / (depth * (CameraInfo.y - CameraInfo.x) + CameraInfo.x);
}

float4 main(float2 uv : Texcoord) : SV_Target
{
    //tex.GetDimensions(width, height);
	float3 color = 0;
	float alpha = 0;
	[brench]if (uv.x < 1 / 4.0f && uv.y < 1 / 4.0f)
	{
		color = gbuffer[0].Sample(splr, uv * 4, 0).rgb;
		alpha = 1;
	}
	else if (1 / 4.0f <= uv.x && uv.x < 2 / 4.0f && uv.y < 1 / 4.0f)
	{
		color = gbuffer[0].Sample(splr, float2(uv.x - 0.25f * 1, uv.y) * 4).aaa;
		alpha = 1;
	}
	else if (2 / 4.0f <= uv.x && uv.x < 3 / 4.0f && uv.y < 1 / 4.0f)
	{
		color = gbuffer[1].Sample(splr, float2(uv.x - 0.25f * 2, uv.y) * 4).rrr;
		alpha = 1;
	}
	else if (3 / 4.0f <= uv.x && uv.x <= 1.0f && uv.y < 1 / 4.0f)
	{
		color = gbuffer[1].Sample(splr, float2(uv.x - 0.25f * 3, uv.y) * 4).ggg;
		alpha = 1;
	}
	else if (3 / 4.0f <= uv.x && uv.x <= 1.0f && 1 / 4.0f < uv.y && uv.y < 2 / 4.0f)
	{
		color = gbuffer[1].Sample(splr, float2(uv.x - 0.25f * 3, uv.y - 0.25f * 1) * 4).bbb;
		alpha = 1;
	}
	else if (3 / 4.0f <= uv.x && uv.x <= 1.0f && 2 / 4.0f < uv.y && uv.y < 3 / 4.0f)
	{
		color = gbuffer[1].Sample(splr, float2(uv.x - 0.25f * 3, uv.y - 0.25f * 2) * 4).aaa;
		color = round(color * (float) 0xFF) / 16;
		alpha = 1;
	}
	else if (3 / 4.0f <= uv.x && uv.x <= 1.0f && 3 / 4.0f < uv.y && uv.y <= 1.0f)
	{
		color = gbuffer[2].Sample(splr, float2(uv.x - 0.25f * 3, uv.y - 0.25f * 3) * 4).rgb;
		alpha = 1;
	}
	else if (2 / 4.0f <= uv.x && uv.x <= 3 / 4.0f && 3 / 4.0f < uv.y && uv.y <= 1.0f)
	{
		color = gbuffer[3].Sample(splr, float2(uv.x - 0.25f * 2, uv.y - 0.25f * 3) * 4).rgb;
		alpha = 1;
	}
	else if (1 / 4.0f <= uv.x && uv.x <= 2 / 4.0f && 3 / 4.0f < uv.y && uv.y <= 1.0f)
	{
		float sceneZ = depth.Sample(splr, float2(uv.x - 0.25f * 1, uv.y - 0.25f * 3) * 4).rrr;
		sceneZ = ConvertToLinearDepth(sceneZ);
		color = sceneZ;
		alpha = 1;
	}
	return float4(color, 0);
}