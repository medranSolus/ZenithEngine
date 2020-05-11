#include "TransformCBuffer.fx"

cbuffer OffsetBuffer
{
	float offset;
};

float4 main(float3 pos : POSITION, float3 normal : NORMAL) : SV_Position
{
	return mul(float4(pos + normal * offset, 1.0f), transformViewProjection);
}