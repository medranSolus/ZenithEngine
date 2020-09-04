#include "Utils.hlsli"
#include "LightCBuffer.hlsli"

cbuffer PixelBuffer
{
	float3 specularColor;
	float specularIntensity; // The bigger the brighter
	float specularPower;     // The smaller the less focused in one point
#ifdef _TEX
#ifdef _TEX_NORMAL
	float normalMapWeight;
#endif
#ifdef _TEX_SPEC
	bool useSpecularPowerAlpha;
#endif
#else
	float4 materialColor;
#endif
};

#ifdef _TEX
SamplerState splr;
Texture2D tex;
#ifdef _TEX_NORMAL
Texture2D normalMap;
#endif
#ifdef _TEX_SPEC
Texture2D spec : register(t2);
#endif
#endif
SamplerState shadowSplr;
Texture2D shadowMap : register(t3);

float4 main(float3 viewPos : POSITION, float3 viewNormal : NORMAL, float4 shadowPos : SHADOW_POSITION
#ifdef _TEX
	, float2 tc : TEXCOORD
#ifdef _TEX_NORMAL
	, float3 viewTan : TANGENT,
	float3 viewBitan : BITANGENT
#endif
#endif
) : SV_Target
{
#ifdef _TEX
	const float4 color = tex.Sample(splr, tc);
#else
	const float4 color = materialColor;
#endif
	clip(color.a - 0.0039f);

	float3 diffuse, specular;
	shadowPos.xyz /= shadowPos.w;

	// Shadow test
	if (shadowMap.Sample(shadowSplr, shadowPos.xy).r > shadowPos.z - 0.001f)
	{
#ifdef _TEX_NORMAL
		viewNormal = lerp(viewNormal,
			GetMappedNormal(viewTan, viewBitan, viewNormal, tc, normalMap, splr), normalMapWeight); // TODO: Add this
#else
		viewNormal = normalize(viewNormal);
#endif
		// Only for double sided objects
		//if (dot(viewNormal, viewPos) >= 0.0f)
		//	viewNormal *= -1.0f;
		LightVectorData lightVD = GetLightVectorData(lightPos, viewPos);

		const float attenuation = GetAttenuation(atteuationConst, atteuationLinear, attenuationQuad, lightVD.distanceToLight);
		const float3 scaledLightColor = lightColor * lightIntensity * attenuation;
		diffuse = GetDiffuse(scaledLightColor, lightVD.directionToLight, viewNormal, attenuation);

		float3 specColor;
		float specPower;
#ifdef _TEX_SPEC
		const float4 specularTex = spec.Sample(splr, tc);
		if (useSpecularPowerAlpha)
			specPower = GetSampledSpecularPower(specularTex);
		specColor = specularTex.rgb;
#else
		specColor = specularColor;
		specPower = specularPower;
#endif
		specular = GetSpecular(lightVD.vertexToLight, viewPos, viewNormal, attenuation, diffuse * specColor, specularPower, specularIntensity);
	}
	else
		diffuse = specular = float3(0.0f, 0.0f, 0.0f);

	return float4(saturate((diffuse + ambientColor) * color.rgb + specular), color.a);
}