#include "CommonUtils.hlsli"
#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "WorldDataCB.hlsli"
#include "Utils/LightUtils.hlsli"
#include "CB/PointLight.hlsli"

TextureCube shadowMap   : register(t0);
Texture2D   normalMap   : register(t1);
Texture2D   specularMap : register(t2); // RGB - color, A - power
Texture2D   depthMap    : register(t3);

struct PSOut
{
	float4 color    : SV_TARGET0;
	float4 specular : SV_TARGET1;
};

PSOut main(float3 texPos : TEX_POSITION)
{
	// Position depth reconstruction
	const float2 tc = float2(0.5f, -0.5f) * (texPos.xy / texPos.z) + 0.5f;
	const float3 position = GetWorldPosition(tc, depthMap.Sample(splr_PR, tc).x, cb_worldData.ViewProjectionInverse);

	// Compute main colors and direction
	const float3 shadowColor = DeleteGammaCorr(cb_light.ShadowColor);
	float3 directionToLight = cb_lightPos - position;
	const float lightDistance = length(directionToLight);
	const float3 lightColor = DeleteGammaCorr(cb_light.Color) * (cb_light.Intensity / GetAttenuation(cb_light.AttnLinear, cb_light.AttnQuad, lightDistance));
	directionToLight /= lightDistance;

	PSOut pso;

	// Shadow test
	const float shadowLevel = GetShadowLevel(normalize(cb_worldData.CameraPos - position), lightDistance, directionToLight, splr_AR, shadowMap, cb_pbrData.ShadowMapSize);
	if (shadowLevel != 0.0f)
	{
		const float3 normal = DecodeNormal(normalMap.Sample(splr_PR, tc).rg);
		pso.color = float4(lerp(shadowColor, GetDiffuse(lightColor, directionToLight, normal), shadowLevel), 0.0f);

		if (shadowLevel > 0.98f)
		{
			const float4 specularData = specularMap.Sample(splr_PR, tc);
			pso.specular = float4(GetSpecular(cb_worldData.CameraPos, directionToLight, position, normal,
				pso.color.rgb * specularData.rgb, specularData.a), 0.0f);
		}
		else
			pso.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else
	{
		pso.color = float4(-shadowColor, 0.0f);
		pso.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	return pso;
}