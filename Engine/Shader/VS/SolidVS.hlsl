#include "CB/Transform.hlsli"

float4 main(/*float3 pos : POSITION*/) : SV_POSITION
{
	float3 pos = 0.0f;
	return mul(float4(pos, 1.0f), cb_transform.MVP);
}