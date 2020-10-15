#include "LightUtilsPS.hlsli"
#include "PointLightPB.hlsli"
#include "HDRGammaPB.hlsli"
#include "CameraPB.hlsli"
#include "BiasPB.hlsli"

SamplerState splr     : register(s0);
SamplerState gbufSplr : register(s1);

Texture2D colorTex    : register(t4); // RGB - color, A = 0.0f: solid; 0.5f: light source; 1.0f: normal
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
	const float3 lightColor = DeleteGammaCorr(cb_lightColor);

	const float isSolid = colorTex.Sample(gbufSplr, tc).a;
	if (isSolid == 0.0f)
	{
		const float3 normal = DecodeNormal(normalTex.Sample(gbufSplr, tc).rg);
		// Shadow test (cb_mapSize from BiasPB is bound implicitly from Shadow Mapping Pass since it has to be run always before Lighting Pass)
		const float shadowLevel = GetShadowLevel(position, normal, lightVD, cb_lightPos, splr, shadowMap, cb_mapSize);
		const float3 shadowColor = DeleteGammaCorr(cb_shadowColor);

		if (shadowLevel != 0.0f)
		{
			const float attenuation = GetAttenuation(cb_atteuationConst, cb_atteuationLinear, cb_attenuationQuad, lightVD);
			const float3 diffuse = GetDiffuse(lightColor, lightVD, normal, attenuation * cb_lightIntensity);
			pso.color = float4(lerp(shadowColor, diffuse, shadowLevel), 1.0f);

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
			pso.color = float4(-shadowColor, 0.0f);
			pso.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	else if (isSolid == 1.0f)
	{
		const float attenuation = GetAttenuation(cb_atteuationConst, cb_atteuationLinear, cb_attenuationQuad, lightVD);
		pso.color = float4(lightColor * (attenuation * cb_lightIntensity), 0.0f);
		pso.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else
	{
		pso.color = float4(lightColor * cb_lightIntensity, 0.0f);
		pso.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	return pso;
}