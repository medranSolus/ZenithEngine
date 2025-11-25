#include "GBufferUtils.hlsli"
#include "Samplers.hlsli"
#include "SettingsDataCB.hlsli"
#include "DynamicDataCB.hlsli"
#include "Utils/LightUtils.hlsli"
#include "Utils/PbrUtils.hlsli"

TEX2D(directLight, 0, 2);
#ifdef _ZE_LIGHT_COMBINE_AO
TEXTURE_EX(ssaoMap, Texture2D<uint>, 1, 2);
#endif
#ifdef _ZE_LIGHT_COMBINE_IBL
TEXTURE_EX(irrMap, TextureCube, 2, 3);
TEXTURE_EX(envMap, TextureCube, 3, 3);
TEX2D(brdfLUT, 4, 3);
TEX2D(depthMap, 5, 4);
TEXTURE_EX(normalMap,      Texture2D<CodedNormalGB>,    6, 4);
TEXTURE_EX(albedo,         Texture2D<AlbedoGB>,         7, 4);
TEXTURE_EX(materialParams, Texture2D<PackedMaterialGB>, 8, 4);
#endif

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
#ifdef _ZE_LIGHT_COMBINE_IBL
	
	const float3 position = GetWorldPosition(tc, tx_depthMap.Sample(splr_PR, tc).x, cb_dynamicData.ViewProjectionInverse);
	const float3 normal = DecodeNormal(tx_normalMap.Sample(splr_PR, tc));
	const float3 albedo = tx_albedo.Sample(splr_PR, tc).rgb;
	const PackedMaterialGB materialData = tx_materialParams.Sample(splr_PR, tc);
	
	const float3 directionToCamera = normalize(cb_dynamicData.CameraPos - position);
	
	const float3 fresnel = GetFresnelSchlickIBL(normal, directionToCamera, lerp(0.04f, albedo, GetMetalness(materialData)), GetRoughness(materialData)); 
	const float3 irradiance = tx_irrMap.Sample(splr_PR, normal).rgb;
	
	float3 ambient = (float3(1.0f, 1.0f, 1.0f) - fresnel) * irradiance * albedo; 
#else
	float3 ambient = DeleteGammaCorr(cb_settingsData.AmbientLight);
#endif
#ifdef _ZE_LIGHT_COMBINE_AO
	// AO should only affect ambient, GI, and IBL lighting
	const float ssao = tx_ssaoMap[tc * cb_settingsData.RenderSize] / 255.0f;
	ambient *= ssao;
#endif
	return float4(ambient + tx_directLight.Sample(splr_PR, tc).rgb, 1.0f);
}