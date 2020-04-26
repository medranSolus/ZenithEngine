#include "Utils.fx"
#include "LightCBuffer.fx"

cbuffer PixelBuffer
{
	float3 specularColor;
	float specularIntensity; // The bigger the brighter
	float specularPower;     // The smaller the less focused in one point
#ifdef _TEX
	float normalMapWeight;
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

float4 main(float3 viewPos : POSITION, float3 viewNormal : NORMAL
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

	const float attenuation = 1.0f / GetAttenuation(atteuationConst, atteuationLinear, attenuationQuad, lightVD.distanceToLight);
	const float3 scaledLightColor = lightColor * lightIntensity * attenuation;
	const float3 diffuse = GetDiffuse(scaledLightColor, lightVD.directionToLight, viewNormal);

	float3 specColor = specularColor * attenuation;
#ifdef _TEX_SPEC
	const float4 specularTex = spec.Sample(splr, tc);
	const float specularPower = GetSampledSpecularPower(specularTex);
	specColor *= specularTex.rgb;
#endif
	const float3 specular = GetSpecular(lightVD.vertexToLight, viewPos, viewNormal, specColor, specularPower, specularIntensity);

	return float4(saturate((diffuse + ambientColor) * color.rgb + specular), color.a);
}