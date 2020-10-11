#include "LightUtilsPS.hlsli"
#include "PointLightPB.hlsli"
#include "CameraPB.hlsli"

SamplerState splr     : register(s0);
SamplerState gbufSplr : register(s1);

Texture2D colorTex    : register(t4); // RGB - color, A = 0.0f
Texture2D normalTex   : register(t5); // RG - normal
Texture2D specularTex : register(t6); // RGB - color, A - power

TextureCube shadowMap : register(t7);
Texture2D depthMap    : register(t8);

struct PSOut
{
	float4 color : SV_TARGET0;
	float4 specular : SV_TARGET1;
};

PSOut main(float2 tc : TEXCOORD)
{
	PSOut pso;
	tc = saturate(tc);
	const float3 position = GetWorldPosition(tc, depthMap.Sample(splr, tc).x, cb_inverseViewProjection);
	LightVectorData lightVD = GetLightVectorData(cb_lightPos, position);

	const float isSolid = colorTex.Sample(gbufSplr, tc).a;
	if (isSolid == 0.0f)
	{
		const float3 normal = DecodeNormal(normalTex.Sample(gbufSplr, tc).rg);
		// Shadow test
		const float shadowLevel = GetShadowLevel(position, normal, lightVD, cb_lightPos, splr, shadowMap);

		if (shadowLevel != 0.0f)
		{
			const float attenuation = GetAttenuation(cb_atteuationConst, cb_atteuationLinear, cb_attenuationQuad, lightVD);
			const float3 diffuse = GetDiffuse(cb_lightColor, lightVD, normal, attenuation * cb_lightIntensity);
			pso.color = float4(lerp(cb_shadowColor, diffuse, shadowLevel), 1.0f);

			if (shadowLevel > 0.98f && isSolid == 0.0f)
			{
				const float4 specularData = specularTex.Sample(gbufSplr, tc);
				pso.specular = float4(GetSpecular(cb_cameraPos, lightVD, position, normal,
					pso.color.rgb * specularData.rgb, GetSampledSpecularPower(specularData)), 1.0f);
			}
			else
				pso.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		else
		{
			pso.color = float4(-cb_shadowColor, 0.0f);
			pso.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	else
	{
		const float attenuation = GetAttenuation(cb_atteuationConst, cb_atteuationLinear, cb_attenuationQuad, lightVD);
		pso.color = float4(cb_lightColor * (attenuation * cb_lightIntensity), 0.0f);
		pso.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	return pso;
}