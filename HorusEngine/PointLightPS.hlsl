#include "UtilsPS.hlsli"
#include "LightCBuffer.hlsli"

cbuffer CameraBuffer : register (b2)
{
	matrix cb_inverseViewProjection;
	float3 cb_cameraPos;
};

SamplerState splr : register(s0);
SamplerComparisonState shadowSplr : register(s1);

Texture2D colorTex    : register(t4);
Texture2D specularTex : register(t5); // RGB - color, A - 1=no_solid
Texture2D normalTex   : register(t6); // RGB - normal, A - specular power
TextureCube shadowMap : register(t7);
Texture2D depthMap    : register(t8);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 color = colorTex.Sample(splr, tc);
	const float4 specularData = specularTex.Sample(splr, tc);

	const float3 position = GetWorldPosition(tc, depthMap.Sample(splr, tc).x, cb_inverseViewProjection);
	const float shadowLevel = GetShadowLevel(position, cb_lightPos, shadowSplr, shadowMap);

	if (specularData.a == 1.0f)
	{
		const float4 normal = normalTex.Sample(splr, tc);
		LightVectorData lightVD = GetLightVectorData(cb_lightPos, position.rgb);

		// Shadow test
		float3 diffuse, specular;
		if (shadowLevel != 0.0f)
		{
			const float attenuation = GetAttenuation(cb_atteuationConst, cb_atteuationLinear, cb_attenuationQuad, lightVD.distanceToLight);
			diffuse = lerp(cb_shadowColor, GetDiffuse(cb_lightColor * cb_lightIntensity * attenuation,
				lightVD.directionToLight, normal.rgb, attenuation), shadowLevel);
			if (shadowLevel > 0.998f)
				specular = GetSpecular(cb_cameraPos, lightVD.directionToLight, position, normal.rgb, attenuation, diffuse * specularData.rgb, normal.a);
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
		return float4(lerp(cb_shadowColor, color.rgb, shadowLevel), color.a);
}