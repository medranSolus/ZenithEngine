#include "UtilsPS.hlsli"
#include "LightCBuffer.hlsli"

cbuffer PixelBuffer : register(b1)
{
	float3 cb_specularColor;
	float cb_specularIntensity; // The bigger the brighter
	float cb_specularPower;     // The smaller the less focused in one point
#ifdef _TEX
#ifdef _TEX_NORMAL
	float cb_normalMapWeight;
#endif
#ifdef _TEX_SPEC
	bool cb_useSpecularPowerAlpha;
#endif
#else
	float4 cb_materialColor;
#endif
};

#ifdef _TEX
SamplerState splr : register(s0);
Texture2D tex : register(t0);
#ifdef _TEX_NORMAL
Texture2D normalMap : register(t1);
#endif
#ifdef _TEX_SPEC
Texture2D spec : register(t2);
#endif
#endif
SamplerComparisonState shadowSplr : register(s1);
TextureCube shadowMap : register(t3);

float4 main(float3 viewPos : POSITION, float3 viewNormal : NORMAL, float3 shadowPos : SHADOW_POSITION
#ifdef _TEX
	, float2 tc : TEXCOORD
#ifdef _TEX_NORMAL
	, float3 viewTan : TANGENT,
	float3 viewBitan : BITANGENT
#endif
#endif
) : SV_TARGET
{
#ifdef _TEX
	const float4 color = tex.Sample(splr, tc);
#else
	const float4 color = cb_materialColor;
#endif
	clip(color.a - 0.0039f);

	float3 diffuse, specular;
	// Shadow test
	const float shadowLevel = GetShadowLevel(shadowPos, shadowSplr, shadowMap);
	if (shadowLevel != 0.0f)
	{
#ifdef _TEX_NORMAL
		viewNormal = lerp(viewNormal,
			GetMappedNormal(viewTan, viewBitan, viewNormal, tc, normalMap, splr), cb_normalMapWeight); // TODO: Add this
#else
		viewNormal = normalize(viewNormal);
#endif
		// Only for double sided objects
		//if (dot(viewNormal, viewPos) >= 0.0f)
		//	viewNormal *= -1.0f;
		LightVectorData lightVD = GetLightVectorData(cb_lightPos, viewPos);

		const float attenuation = GetAttenuation(cb_atteuationConst, cb_atteuationLinear, cb_attenuationQuad, lightVD.distanceToLight);
		const float3 scaledLightColor = cb_lightColor * cb_lightIntensity * attenuation;
		diffuse = lerp(cb_shadowColor, GetDiffuse(scaledLightColor, lightVD.directionToLight, viewNormal, attenuation), shadowLevel);

		if (shadowLevel >= 0.999999f)
		{
			float3 specColor;
			float specPower;
#ifdef _TEX_SPEC
			const float4 specularTex = spec.Sample(splr, tc);
			if (cb_useSpecularPowerAlpha)
				specPower = GetSampledSpecularPower(specularTex);
			else
				specPower = cb_specularPower;
			specColor = specularTex.rgb;
#else
			specColor = cb_specularColor;
			specPower = cb_specularPower;
#endif
			specular = GetSpecular(lightVD.vertexToLight, viewPos, viewNormal, attenuation, diffuse * specColor, specPower, cb_specularIntensity);
		}
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