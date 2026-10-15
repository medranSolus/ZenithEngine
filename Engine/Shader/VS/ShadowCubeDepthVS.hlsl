#define ZE_BBQ_CB_RANGE 4
#include "CB/ModelGeometryIO.hlsli"
#define ZE_TRANSFORM_CB_RANGE 5
#include "TransformCB.hlsli"

float3 main(PackedPos pospack) : WORLD_POSITION
{
	return mul(float4(GetPosition(pospack), 1.0f), cb_transform).xyz;
}