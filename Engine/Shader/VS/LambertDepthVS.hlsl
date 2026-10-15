#define ZE_BBQ_CB_RANGE 4
#include "CB/ModelGeometryIO.hlsli"
#include "CB/ModelTransform.hlsli"

float4 main(PackedPos pospack) : SV_POSITION
{
	return mul(float4(GetPosition(pospack), 1.0f), cb_transform.MVP);
}