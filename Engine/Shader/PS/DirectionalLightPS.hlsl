#include "GBufferUtils.hlsli"
#include "Samplers.hlsli"
#include "SettingsDataCB.hlsli"
#include "DynamicDataCB.hlsli"
#include "Utils/LightUtils.hlsli"
#include "Utils/PbrUtils.hlsli"
#include "CB/DirectionalLight.hlsli"

TEX2D(shadowMap, 0, 3);
TEX2D(depthMap,  1, 2);
TEXTURE_EX(normalMap,      Texture2D<CodedNormalGB>,    2, 2);
TEXTURE_EX(albedo,         Texture2D<AlbedoGB>,         3, 2);
TEXTURE_EX(materialParams, Texture2D<PackedMaterialGB>, 4, 2);

float4 main(float2 tc : TEXCOORD) : SV_TARGET0
{
	// Position depth reconstruction
	const float3 position = GetWorldPosition(tc, tx_depthMap.Sample(splr_PR, tc).x, cb_dynamicData.ViewProjectionInverse);
	
	// Main light color
	const float3 lightRadiance = cb_light.Color * cb_light.Intensity;
	
	// Compute direction vectors
	const float3 directionToLight = -ct_lightDir.Dir;
	const float3 directionToCamera = normalize(cb_dynamicData.CameraPos - position);
	
	const float3 normal = DecodeNormal(tx_normalMap.Sample(splr_PR, tc));
	const float3 albedo = tx_albedo.Sample(splr_PR, tc).rgb;
	const PackedMaterialGB materialData = tx_materialParams.Sample(splr_PR, tc);
	
	// Get light level and shadow test
	const float3 reflectance = GetBRDFCookTorrance(directionToLight, directionToCamera, normal, albedo, GetMetalness(materialData), GetRoughness(materialData));
	const float shadowLevel = 1.0f;
	//const float shadowLevel = GetShadowLevel(normalize(cb_dynamicData.CameraPos - position), lightDistance,
	//	directionToLight, GetShadowUV(position), splr_AB, tx_shadowMap, cb_settingsData.ShadowMapSize);

	return float4(GetRadiance(lightRadiance, reflectance, shadowLevel), 0.0f);
}