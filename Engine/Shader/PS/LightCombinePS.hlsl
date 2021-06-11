#include "Utils/Samplers.hlsli"
#include "CBuffer/LightAmbient.hlsli"
#include "HDRGammaCB.hlsli"
#include "GBuffer.hlsli"
#include "LightBuffer.hlsli"
#include "SSAOMap.hlsli"

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 srgb = tx_color.Sample(splr_PW, tc);
	const float3 color = DeleteGammaCorr(srgb.rgb);
	const float ssao = tx_ssao.Sample(splr_LW, tc).r;
	const float3 diffuse = ssao * color * (DeleteGammaCorr(cb_ambientLight) + abs(tx_lighting.Sample(splr_PW, tc).rgb));

	return float4(diffuse + tx_specular.Sample(splr_PW, tc).rgb, srgb.a);
}