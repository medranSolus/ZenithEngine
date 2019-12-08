cbuffer TransformConstatBuffer
{
    matrix modelView;
    matrix modelViewProjection;
};

struct VSOut
{
    float2 texc : TexCoord;
    float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float2 texc : TexCoord)
{
	VSOut vout;
	vout.texc = texc;
    vout.pos = mul(modelViewProjection, float4(pos, 1.0f));
	return vout;
}