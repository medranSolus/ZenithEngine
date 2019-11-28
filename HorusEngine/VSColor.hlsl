struct VSOut
{
	float4 col : Color;
	float4 pos : SV_Position;
};

cbuffer ConstantBuffer
{
	matrix transform;
};

VSOut main(float3 pos : Position, float4 color : Color)
{
	VSOut vout;
	vout.col = color;
	vout.pos = mul(float4(pos, 1.0f), transform);
	return vout;
}