#include "LightAmbientPB.hlsli"
#include "HDRGammaPB.hlsli"

SamplerState splr : register(s1);

Texture2D colorTex    : register(t4);  // RGB - color, A = 0.0f

Texture2D lightingMap : register(t9);  // RGB - light color
Texture2D specularMap : register(t10); // RGB - specular color
Texture2D ssaoMap     : register(t11); // R   - ambient occlusion value

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 srgb = colorTex.Sample(splr, tc);
	const float3 color = DeleteGammaCorr(srgb.rgb);
	const float ssao = ssaoMap.Sample(splr, tc).r;
	const float3 diffuse = ssao * color * (DeleteGammaCorr(cb_ambientLight) + lightingMap.Sample(splr, tc).rgb);

	return float4(diffuse + specularMap.Sample(splr, tc).rgb, srgb.a);
}