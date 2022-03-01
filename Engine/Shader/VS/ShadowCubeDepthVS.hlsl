#include "TransformCB.hlsli"

float3 main(float3 pos : POSITION) : POSITION
{
	return mul(float4(pos, 1.0f), cb_transform.Transforms[cb_transformIndex]).xyz;
}