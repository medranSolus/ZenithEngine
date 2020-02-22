#include "SolidCBuffer.fx"

float4 main() : SV_Target
{
	clip(materialColor.a < 0.005f ? -1 : 1);
	return materialColor;
}