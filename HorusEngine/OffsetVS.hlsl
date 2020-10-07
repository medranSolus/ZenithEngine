#include "TransformVB.hlsli"
#include "OffsetVB.hlsli"

float4 main(float3 pos : POSITION, float3 normal : NORMAL) : SV_POSITION
{
	return mul(float4(pos + normal * cb_offset, 1.0f), cb_transformViewProjection);
}