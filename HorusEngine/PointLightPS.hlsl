#include "UtilsPS.hlsli"
#include "LightCBuffer.hlsli"

SamplerState splr : register(s0);
SamplerComparisonState shadowSplr : register(s1);

Texture2D colorTex    : register(t0);
Texture2D specularTex : register(t1); // RGB - color, A - 1=no_solid
Texture2D positionTex : register(t2); // RGB - position, A - specular intensity
Texture2D normalTex   : register(t3); // RGB - normal, A - power
TextureCube shadowMap : register(t4);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 color = colorTex.Sample(splr, tc);
	const float4 specularData = specularTex.Sample(splr, tc);
	if (specularData.a == 1.0f)
	{
		const float4 position = positionTex.Sample(splr, tc);
		const float4 normal = normalTex.Sample(splr, tc);
		LightVectorData lightVD = GetLightVectorData(cb_lightPos, position.rgb);

		// Shadow test
		const float shadowLevel = 1.0f;//GetShadowLevel(shadowPos, shadowSplr, shadowMap);

		const float attenuation = GetAttenuation(cb_atteuationConst, cb_atteuationLinear, cb_attenuationQuad, lightVD.distanceToLight);
		float3 diffuse = lerp(cb_shadowColor, GetDiffuse(cb_lightColor * cb_lightIntensity * attenuation,
			lightVD.directionToLight, normal.rgb, attenuation), shadowLevel);
		float3 specular = GetSpecular(lightVD.vertexToLight, position.rgb, normal.rgb, attenuation, diffuse * specularData.rgb, normal.a, position.a);

		return float4(saturate(diffuse + cb_ambientColor) * color.rgb + specular, color.a);
	}
	else
		return color;
}