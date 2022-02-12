#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"

Texture2D ssaoMap  : register(t0);
Texture2D colorMap : register(t1);
Texture2D lighting : register(t2);
Texture2D specular : register(t3);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 srgb = colorMap.Sample(splr_PW, tc);
	const float3 color = DeleteGammaCorr(srgb.rgb);
	const float ssao = ssaoMap.Sample(splr_LW, tc).r;
	const float3 diffuse = ssao * color * (DeleteGammaCorr(cb_pbrData.AmbientLight) + abs(lighting.Sample(splr_PW, tc).rgb));

	return float4(diffuse + specular.Sample(splr_PW, tc).rgb, srgb.a);
}