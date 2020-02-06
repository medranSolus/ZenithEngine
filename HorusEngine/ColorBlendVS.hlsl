cbuffer TransformConstatBuffer
{
	matrix transformView;
	matrix scalingTransformView;
	matrix scalingTransformViewProjection;
};

struct VSOut
{
	float4 col : COLOR;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : POSITION, float4 color : COLOR)
{
	VSOut vso;
	vso.col = color;
	vso.pos = mul(float4(pos, 1.0f), scalingTransformViewProjection);
	return vso;
}