cbuffer TransformConstatBuffer
{
	matrix transform;
	matrix scaling;
	matrix view;
	matrix projection;
};

struct VSOut
{
	float4 col : COLOR;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : POSITION, float4 color : COLOR)
{
	VSOut vout;
	vout.col = color;
	vout.pos = mul(float4(pos, 1.0f), mul(mul(mul(scaling, transform), view), projection));
	return vout;
}