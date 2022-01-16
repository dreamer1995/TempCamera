struct VSOut
{
    float2 uv : Texcoord;
	nointerpolation uint sliceId : SLICEINDEX;
    float4 pos : SV_Position;
};

VSOut main(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID, float2 pos : Position)
{
    VSOut vso;
    vso.pos = float4(pos, 0.0f, 1.0f);
    vso.uv = float2((pos.x + 1) / 2.0f, -(pos.y - 1) / 2.0f);
	vso.sliceId = instanceId;
    return vso;
}