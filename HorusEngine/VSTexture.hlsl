struct VSOut
{
	float2 texc : TexCoord;
	float4 pos : SV_Position;
};

cbuffer ConstantBuffer
{
	matrix transform;
};

VSOut main(float3 pos : Position, float2 texc : TexCoord)
{
	VSOut vout;
	vout.texc = texc;
	vout.pos = mul(transform, float4(pos, 1.0f));
	return vout;
}