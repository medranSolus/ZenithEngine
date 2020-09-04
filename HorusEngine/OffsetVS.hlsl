#include "TransformCBuffer.hlsli"

cbuffer OffsetBuffer
{
	float cb_offset;
};

float4 main(float3 pos : POSITION, float3 normal : NORMAL) : SV_Position
{
	return mul(float4(pos + normal * cb_offset, 1.0f), cb_transformViewProjection);
}