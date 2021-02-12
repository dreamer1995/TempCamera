cbuffer TransformCBuf : register(b13) //VS, PS
{
	matrix matrix_I_P;
};

struct VSOut
{
    float2 uv : Texcoord;
	float3 viewRay : Texcoord1;
    float4 pos : SV_Position;
};

VSOut main(float2 pos : Position)
{
    VSOut vso;
    vso.pos = float4(pos, 0.0f, 1.0f);
    vso.uv = float2((pos.x + 1) / 2.0f, -(pos.y - 1) / 2.0f);
	float3 positionVS = mul(vso.pos.xyz, matrix_I_P);
	vso.viewRay = positionVS;
    return vso;
}