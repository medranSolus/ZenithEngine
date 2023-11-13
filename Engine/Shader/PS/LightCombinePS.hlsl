#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"

TEX2D(colorMap, 0, 1);
TEX2D(lighting, 1, 2);
TEX2D(specular, 2, 2);
#ifdef _ZE_LIGHT_COMBINE_AO
TEXTURE_EX(ssaoMap, Texture2D<uint>, 3, 3);
#endif

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 srgb = tx_colorMap.Sample(splr_PR, tc);
	const float3 color = DeleteGammaCorr(srgb.rgb);
	float3 diffuse = color * (DeleteGammaCorr(cb_pbrData.AmbientLight) + tx_lighting.Sample(splr_PR, tc).rgb);
#ifdef _ZE_LIGHT_COMBINE_AO
	const float ssao = tx_ssaoMap[tc * cb_pbrData.RenderSize] / 255.0f;
	diffuse *= ssao;
#endif
	return float4(diffuse + tx_specular.Sample(splr_PR, tc).rgb, srgb.a);
}