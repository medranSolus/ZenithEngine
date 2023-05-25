#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"

TEXTURE_EX(ssaoMap, Texture2D<uint>, 0, 0);
TEX2D(colorMap, 1);
TEX2D(lighting, 2);
TEX2D(specular, 3);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 srgb = tx_colorMap.Sample(splr_PR, tc);
	const float3 color = DeleteGammaCorr(srgb.rgb);
	const float ssao = tx_ssaoMap[tc * cb_pbrData.SsaoData.ViewportSize] / 255.0f;
	const float3 diffuse = ssao * color * (DeleteGammaCorr(cb_pbrData.AmbientLight) + tx_lighting.Sample(splr_PR, tc).rgb);

	return float4(diffuse + tx_specular.Sample(splr_PR, tc).rgb, srgb.a);
}