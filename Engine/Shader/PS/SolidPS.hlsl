#include "CB/Solid.hlsli"

float4 main() : SV_TARGET
{
	return float4(ct_solidColor, 1.0f);
}