#include "TransformCBuffer.hlsli"

float4 main(float3 pos : POSITION) : SV_Position
{
	return mul(float4(pos, 1.0f), cb_transformViewProjection);
}