#include "CommonUtils.hlsli"
#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "WorldDataCB.hlsli"
#include "Utils/LightUtils.hlsli"
#include "CB/PointLight.hlsli"

TEXTURE_EX(shadowMap, TextureCube, 0, 0);
TEX2D(normalMap,   1);
TEX2D(specularMap, 2); // RGB - color, A - power
TEX2D(depthMap,    3);

struct PSOut
{
	float4 color    : SV_TARGET0;
	float4 specular : SV_TARGET1;
};

PSOut main(float3 texPos : TEX_POSITION)
{
	// Position depth reconstruction
	const float2 tc = float2(0.5f, -0.5f) * (texPos.xy / texPos.z) + 0.5f;
	const float3 position = GetWorldPosition(tc, tx_depthMap.Sample(splr_PR, tc).x, cb_worldData.ViewProjectionInverse);

	// Compute main colors and direction
	const float3 shadowColor = DeleteGammaCorr(cb_light.ShadowColor);
	float3 directionToLight = ct_lightPos - position;
	const float lightDistance = length(directionToLight);
	const float3 lightColor = DeleteGammaCorr(cb_light.Color) * (cb_light.Intensity / GetAttenuation(cb_light.AttnLinear, cb_light.AttnQuad, lightDistance));
	directionToLight /= lightDistance;

	// Shadow test
	const float shadowLevel = GetShadowLevel(normalize(cb_worldData.CameraPos - position), lightDistance, directionToLight, splr_AR, tx_shadowMap, cb_pbrData.ShadowMapSize);

	const float3 normal = DecodeNormal(tx_normalMap.Sample(splr_PR, tc).rg);
	const float4 specularData = tx_specularMap.Sample(splr_PR, tc);

	PSOut pso;

	pso.color = float4(lerp(shadowColor, GetDiffuse(lightColor, directionToLight, normal), shadowLevel) * float(shadowLevel >= 0.001f), 0.0f);

	pso.specular = float4(GetSpecular(cb_worldData.CameraPos, directionToLight, position, normal,
		pso.color.rgb * specularData.rgb, specularData.a) * float(shadowLevel > 0.98f), 0.0f);

	return pso;
}