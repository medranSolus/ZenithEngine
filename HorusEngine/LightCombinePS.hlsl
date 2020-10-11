#include "LightUtilsPS.hlsli"
#include "LightAmbientPB.hlsli"
#include "HDRGammaPB.hlsli"

SamplerState splr : register(s0);

Texture2D colorTex    : register(t4); // RGB - color, A = 0.0f

Texture2D lightingMap : register(t9); // RGB - light color
Texture2D specularMap : register(t10); // RGB - specular color

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 srgb = colorTex.Sample(splr, tc);
	const float3 color = pow(srgb.rgb, float3(cb_deGamma, cb_deGamma, cb_deGamma));
	const float3 diffuse = cb_ambientLight + lightingMap.Sample(splr, tc).rgb;

	if (srgb.a == 0.0f)
	{
		const float3 specular = specularMap.Sample(splr, tc).rgb;
		return float4(diffuse * color + specular, 1.0f);
	}
	else
		return float4(diffuse * color, 1.0f);
}