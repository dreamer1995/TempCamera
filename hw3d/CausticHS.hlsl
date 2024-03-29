cbuffer CBuf : register(b10)
{
	int tessellation;
};
struct VS_CONTROL_POINT_OUTPUT
{
	float3 vPosition : Position;
	float2 uv : Texcoord;
	// TODO: change/add other stuff
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float3 vPosition : Position;
	float2 uv : Texcoord;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[4]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor[2] : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 4

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	// Insert code to compute Output here
	Output.EdgeTessFactor[0] =
		Output.EdgeTessFactor[1] =
		Output.EdgeTessFactor[2] =
		Output.EdgeTessFactor[3] = tessellation; // e.g. could calculate dynamic tessellation factors instead

	Output.InsideTessFactor[0] =
		Output.InsideTessFactor[1] = tessellation;

	return Output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONTROL_POINT_OUTPUT Output;

	// Insert code to compute Output here
	Output.vPosition = ip[i].vPosition;
	Output.uv = ip[i].uv;
	return Output;
}