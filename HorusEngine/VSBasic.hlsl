cbuffer ConstantBuffer
{
	matrix transform;
};

float4 main(float3 pos : Position) : SV_Position
{
	return mul(transform, float4(pos, 1.0f));
}