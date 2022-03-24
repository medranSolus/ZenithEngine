#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"

TextureCube box : register(t0);

float4 main(float3 worldPos : POSITION) : SV_TARGET
{
	return float4(DeleteGammaCorr(box.Sample(splr_AR, worldPos).rgb), 1.0f);
}