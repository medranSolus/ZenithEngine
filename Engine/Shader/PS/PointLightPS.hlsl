#include "CB/PointLight.hlsli"
#include "DynamicDataCB.hlsli"
#include "GBufferUtils.hlsli"
#include "Samplers.hlsli"
#include "SettingsDataCB.hlsli"
#include "UtilsPS.hlsli"

TEXTURE_EX(shadowMap,      TextureCube,                 0, 3);
TEX2D(depthMap,                                         1, 2);
TEXTURE_EX(normalMap,      Texture2D<CodedNormalGB>,    2, 2);
TEXTURE_EX(albedo,         Texture2D<AlbedoGB>,         3, 2);
TEXTURE_EX(materialParams, Texture2D<PackedMaterialGB>, 4, 2);

float4 main(float3 texPos : TEX_POSITION) : SV_TARGET0
{
	// Position depth reconstruction
	const float2 tc = float2(0.5f, -0.5f) * (texPos.xy / texPos.z) + 0.5f;
	const float3 position = GetWorldPosition(tc, tx_depthMap.Sample(splr_PR, tc).x, cb_dynamicData.ViewProjectionInverse);
	
	// Compute direction vectors
	const float3 directionToCamera = normalize(cb_dynamicData.CameraPos - position);
	float3 directionToLight = ct_lightPos.Pos - position;
	const float lightDistance = length(directionToLight);
	directionToLight /= lightDistance;
	
	// Main light color
	const float3 lightRadiantFlux = cb_light.Color * (cb_light.Intensity / GetAttenuation(cb_light.AttnLinear, cb_light.AttnQuad, lightDistance));
	
	// Get GBuffer params
	const float3 normal = DecodeNormal(tx_normalMap.Sample(splr_PR, tc));
	const float3 albedo = tx_albedo.Sample(splr_PR, tc).rgb;
	const PackedMaterialGB materialData = tx_materialParams.Sample(splr_PR, tc);
	
	// Get light level and shadow test
	const float3 reflectance = GetBRDFCookTorrance(directionToLight, directionToCamera, normal, albedo, GetMetalness(materialData), GetRoughness(materialData));
	const float shadowLevel = GetShadowLevel(directionToCamera, lightDistance, directionToLight, splr_AR, tx_shadowMap, cb_settingsData.ShadowMapSize);

	return float4(GetRadiance(lightRadiantFlux, reflectance, shadowLevel), 0.0f);
}