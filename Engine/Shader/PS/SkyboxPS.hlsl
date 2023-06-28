#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"

TEXTURE_EX(sky, TextureCube, 0, 2);

float4 main(float3 worldPos : POSITION) : SV_TARGET
{
	return float4(DeleteGammaCorr(tx_sky.Sample(splr_AR, worldPos).rgb), 1.0f);
}