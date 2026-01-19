#include "GBufferUtils.hlsli"
#include "Samplers.hlsli"
#include "SettingsDataCB.hlsli"
#include "DynamicDataCB.hlsli"
#define ZE_TRANSFORM_CB_RANGE 6
#include "TransformCB.hlsli"
#include "Utils/LightUtils.hlsli"
#include "Utils/PbrUtils.hlsli"
#include "CB/SpotLight.hlsli"

TEX2D(shadowMap, 0, 3);
TEX2D(depthMap,  1, 2);
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

	// Test if inside angle
	const float theta = dot(directionToLight, -cb_light.Direction);
	const float outer = cos(cb_light.OuterAngle);
	const float isInside = theta > outer;
	
	// Main light color
	const float3 lightRadiance = cb_light.Color * cb_light.Intensity *
		smoothstep(0.0f, 1.0f, (theta - outer) / (cos(cb_light.InnerAngle) - outer)) /
		GetAttenuation(cb_light.AttnLinear, cb_light.AttnQuad, lightDistance);
	
	const float3 normal = DecodeNormal(tx_normalMap.Sample(splr_PR, tc));
	const float3 albedo = tx_albedo.Sample(splr_PR, tc).rgb;
	const PackedMaterialGB materialData = tx_materialParams.Sample(splr_PR, tc);
	
	// Get light level and shadow test
	const float3 reflectance = GetBRDFCookTorrance(directionToLight, directionToCamera, normal, albedo, GetMetalness(materialData), GetRoughness(materialData));
	const float shadowLevel = GetShadowLevel(directionToCamera, lightDistance,
		directionToLight, GetShadowUV(position, cb_transform), splr_AE, tx_shadowMap, cb_settingsData.ShadowMapSize);

	return float4(GetRadiance(lightRadiance, reflectance, shadowLevel) * isInside, 0.0f);
}