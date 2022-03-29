#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"

Texture2D<uint> ssaoMap  : register(t0);
Texture2D colorMap : register(t1);
Texture2D lighting : register(t2);
Texture2D specular : register(t3);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 srgb = colorMap.Sample(splr_PR, tc);
	const float3 color = DeleteGammaCorr(srgb.rgb);
	const float ssao = ssaoMap[tc * cb_pbrData.SsaoData.ViewportSize] / 255.0f;
	const float3 diffuse = ssao * color * (DeleteGammaCorr(cb_pbrData.AmbientLight) + lighting.Sample(splr_PR, tc).rgb);

	return float4(diffuse + specular.Sample(splr_PR, tc).rgb, srgb.a);
}