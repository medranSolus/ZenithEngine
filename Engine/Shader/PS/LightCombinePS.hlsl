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
TEXTURE_EX(envMap, TextureCube, 3, 4);
TEX2D(brdfLUT, 4, 5);
TEX2D(depthMap, 5, 6);
TEXTURE_EX(normalMap,      Texture2D<CodedNormalGB>,    6, 6);
TEXTURE_EX(albedo,         Texture2D<AlbedoGB>,         7, 6);
TEXTURE_EX(materialParams, Texture2D<PackedMaterialGB>, 8, 6);
#endif

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
#ifdef _ZE_LIGHT_COMBINE_IBL
	const float3 position = GetWorldPosition(tc, tx_depthMap.Sample(splr_PR, tc).x, cb_dynamicData.ViewProjectionInverse);
	const float3 normal = DecodeNormal(tx_normalMap.Sample(splr_PR, tc));
	const float3 albedo = tx_albedo.Sample(splr_PR, tc).rgb;
	const PackedMaterialGB materialData = tx_materialParams.Sample(splr_PR, tc);
	
	const float metalness = GetMetalness(materialData);
	const float roughness = GetRoughness(materialData);
	
	const float3 directionToCamera = normalize(cb_dynamicData.CameraPos - position);
	
	const float cosTheta = max(dot(normal, directionToCamera), 0.0f);
	const float3 fresnel = GetFresnelSchlickIBL(cosTheta, lerp(0.04f, albedo, metalness), roughness);
	
	const float3 irradiance = tx_irrMap.Sample(splr_LE, normal).rgb;
	const float3 diffuseLevel = (1.0f - fresnel) * (1.0f - metalness);
	
	uint width, height, mipCount;
	tx_envMap.GetDimensions(0, width, height, mipCount);
	const float3 prefilteredColor = tx_envMap.SampleLevel(splr_LE, normal, roughness * mipCount).rgb;
	const float2 envBRDF  = tx_brdfLUT.Sample(splr_PE, float2(cosTheta, roughness)).rg;
	
	float3 ambient = diffuseLevel * irradiance * albedo + prefilteredColor * (fresnel * envBRDF.x + envBRDF.y);
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