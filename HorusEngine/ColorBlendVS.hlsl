cbuffer TransformConstatBuffer
{
    matrix modelView;
    matrix modelViewProjection;
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
    vout.pos = mul(modelViewProjection, float4(pos, 1.0f));
	return vout;
}