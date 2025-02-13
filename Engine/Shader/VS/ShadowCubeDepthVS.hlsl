#define ZE_TRANSFORM_CB_RANGE 4
#include "TransformCB.hlsli"

float3 main(float3 pos : POSITION) : WORLD_POSITION
{
	return mul(float4(pos, 1.0f), cb_transform).xyz;
}