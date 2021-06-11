#include "CBuffer/Solid.hlsli"

float4 main() : SV_TARGET
{
	return float4(cb_solidColor, 1.0f);
}