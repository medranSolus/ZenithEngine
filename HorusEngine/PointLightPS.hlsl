#include "UtilsPS.hlsli"
#include "LightCBuffer.hlsli"

cbuffer CameraBuffer : register (b2)
{
	float3 cb_cameraPos;
};

SamplerState splr : register(s0);
SamplerComparisonState shadowSplr : register(s1);

Texture2D colorTex    : register(t4);
Texture2D specularTex : register(t5); // RGB - color, A - 1=no_solid
Texture2D positionTex : register(t6); // RGB - position, A - specular intensity
Texture2D normalTex   : register(t7); // RGB - normal, A - power
TextureCube shadowMap : register(t8);
Texture2D depthMap    : register(t9);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float depth = depthMap.Sample(splr, tc).x;
	const float4 color = colorTex.Sample(splr, tc);
	const float4 specularData = specularTex.Sample(splr, tc);

	if (specularData.a == 1.0f)
	{
		const float4 position = positionTex.Sample(splr, tc);
		const float4 normal = normalTex.Sample(splr, tc);
		LightVectorData lightVD = GetLightVectorData(cb_lightPos, position.rgb);

		// Shadow test
		const float shadowLevel = GetShadowLevel(position.rgb, cb_lightPos, shadowSplr, shadowMap);
		float3 diffuse, specular;
		if (shadowLevel != 0.0f)
		{
			const float attenuation = GetAttenuation(cb_atteuationConst, cb_atteuationLinear, cb_attenuationQuad, lightVD.distanceToLight);
			diffuse = lerp(cb_shadowColor, GetDiffuse(cb_lightColor * cb_lightIntensity * attenuation,
				lightVD.directionToLight, normal.rgb, attenuation), shadowLevel);
			if (shadowLevel > 0.998f)
				specular = GetSpecular(cb_cameraPos, lightVD.directionToLight, position.rgb, normal.rgb, attenuation, diffuse * specularData.rgb, normal.a, position.a);
			else
				specular = float3(0.0f, 0.0f, 0.0f);
		}
		else
		{
			diffuse = cb_shadowColor;
			specular = float3(0.0f, 0.0f, 0.0f);
		}
		return float4(saturate(diffuse + cb_ambientColor) * color.rgb + specular, color.a);
	}
	else
		return color;
}