#include "LightUtilsPS.hlsli"
#include "PointLightPB.hlsli"
#include "HDRGammaPB.hlsli"
#include "CameraPB.hlsli"

SamplerState splr : register(s0);

Texture2D colorTex    : register(t4); // RGB - color, A = 0.0f
Texture2D normalTex   : register(t5); // RG - normal
Texture2D specularTex : register(t6); // RGB - color, A - power

TextureCube shadowMap : register(t7);
Texture2D depthMap    : register(t8);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 srgb = colorTex.Sample(splr, tc);
	const float3 color = pow(srgb.rgb, float3(cb_deGamma, cb_deGamma, cb_deGamma));
	if (srgb.a == 0.0f)
	{
		const float3 position = GetWorldPosition(tc, depthMap.Sample(splr, tc).x, cb_inverseViewProjection);
		const float3 normal = DecodeNormal(normalTex.Sample(splr, tc).rg);
		LightVectorData lightVD = GetLightVectorData(cb_lightPos, position);

		// Shadow test
		const float shadowLevel = GetShadowLevel(position, normal, lightVD.directionToLight, cb_lightPos, splr, shadowMap);

		float3 diffuse, specular;
		if (shadowLevel != 0.0f)
		{
			const float attenuation = GetAttenuation(cb_atteuationConst, cb_atteuationLinear, cb_attenuationQuad, lightVD.distanceToLight);
			diffuse = lerp(cb_shadowColor, GetDiffuse(cb_lightColor * cb_lightIntensity * attenuation,
				lightVD.directionToLight, normal, attenuation), shadowLevel);
			if (shadowLevel > 0.98f)
			{
				const float4 specularData = specularTex.Sample(splr, tc);
				specular = GetSpecular(cb_cameraPos, lightVD.directionToLight, position, normal,
					attenuation, diffuse * specularData.rgb, GetSampledSpecularPower(specularData));
			}
			else
				specular = float3(0.0f, 0.0f, 0.0f);
		}
		else
		{
			diffuse = cb_shadowColor;
			specular = float3(0.0f, 0.0f, 0.0f);
		}
		return float4((diffuse + cb_ambientColor) * color + specular, 1.0f);
	}
	else
		return float4(color * cb_lightIntensity, 1.0f);
}