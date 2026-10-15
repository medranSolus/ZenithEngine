#define ZE_BBQ_CB_RANGE 1
#include "CB/ModelGeometryIO.hlsli"
#define ZE_TRANSFORM_CB_RANGE 0
#include "TransformCB.hlsli"

float4 main(PackedPos pospack) : SV_POSITION
{
	return mul(float4(GetPosition(pospack), 1.0f), cb_transform);
}