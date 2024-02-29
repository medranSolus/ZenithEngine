#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"

TEX2D(directLight, 0, 1);
#ifdef _ZE_LIGHT_COMBINE_AO
TEXTURE_EX(ssaoMap, Texture2D<uint>, 1, 1);
#endif

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	float3 ambient = DeleteGammaCorr(cb_pbrData.AmbientLight);
#ifdef _ZE_LIGHT_COMBINE_AO
	// AO should only affect ambient, GI, and IBL lighting
	const float ssao = tx_ssaoMap[tc * cb_pbrData.RenderSize] / 255.0f;
	ambient *= ssao;
#endif
	return float4(ambient + tx_directLight.Sample(splr_PR, tc).rgb, 1.0f);
}