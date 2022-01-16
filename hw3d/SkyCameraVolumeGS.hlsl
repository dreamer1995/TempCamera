struct VSOut
{
	float2 uv : Texcoord;
	nointerpolation uint sliceId : SLICEINDEX;
	float4 pos : SV_Position;
};

struct GeometryOutput
{
	float2 uv : Texcoord;
	nointerpolation uint sliceId : SV_RenderTargetArrayIndex; //write to a specific slice, it can also be read in the pixel shader.
	float4 pos : SV_Position;
};

[maxvertexcount(3)]
void main(triangle VSOut input[3], inout TriangleStream<GeometryOutput> gsout)
{
	GeometryOutput output;
	for (uint i = 0; i < 3; i++)
	{
		output.pos = input[i].pos;
		output.uv = input[i].uv;
		output.sliceId = input[0].sliceId;
		gsout.Append(output);
	}
	gsout.RestartStrip();
}