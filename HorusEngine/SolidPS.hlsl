#include "SolidPB.hlsli"

float4 main() : SV_TARGET
{
	return float4(cb_solidColor, cb_isLight ? 0.5f : 1.0f);
}