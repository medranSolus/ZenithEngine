#include "DynamicDataCB.hlsli"
#include "GBufferUtils.hlsli"
#include "Samplers.hlsli"
#include "SettingsDataCB.hlsli"
#include "UtilsPS.hlsli"

TEX2D(directLight, 0, 2);
#ifdef _ZE_LIGHT_COMBINE_AO
#	define SSR_SLOT 2
TEXTURE_EX(ssaoMap, Texture2D<uint>, 1, 2);
#else
#	define SSR_SLOT 1
#endif
#ifdef _ZE_LIGHT_COMBINE_SSR
TEX2D(ssr, SSR_SLOT, 2);
#endif
#if defined(_ZE_LIGHT_COMBINE_IBL) || defined(_ZE_LIGHT_COMBINE_SSR)
TEX2D(depthMap, 3, 3);
TEXTURE_EX(normalMap,      Texture2D<CodedNormalGB>,    4, 3);
TEXTURE_EX(albedo,         Texture2D<AlbedoGB>,         5, 3);
TEXTURE_EX(materialParams, Texture2D<PackedMaterialGB>, 6, 3);
TEXTURE_EX(envMap, TextureCube, 7, 4);
TEX2D(brdfLUT, 8, 5);
#endif
#ifdef _ZE_LIGHT_COMBINE_IBL
TEXTURE_EX(irrMap, TextureCube, 9, 6);
#endif

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
#if defined(_ZE_LIGHT_COMBINE_IBL) || defined(_ZE_LIGHT_COMBINE_SSR)
	const float3 position = GetWorldPosition(tc, tx_depthMap.Sample(splr_PR, tc).x, cb_dynamicData.ViewProjectionInverse);
	const float3 normal = DecodeNormal(tx_normalMap.Sample(splr_PR, tc));
	const float3 albedo = tx_albedo.Sample(splr_PR, tc).rgb;
	const PackedMaterialGB materialData = tx_materialParams.Sample(splr_PR, tc);
	
	const float metalness = GetMetalness(materialData);
	const float roughness = GetRoughness(materialData);
	
	const float3 directionToCamera = normalize(cb_dynamicData.CameraPos - position);
	
	const float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);
	const float cosTheta = max(dot(normal, directionToCamera), 0.0f);
	const float3 fresnel = GetFresnelSchlickIBL(cosTheta, f0, roughness);
	
	float3 ambient = 0.0f;
	
#	ifdef _ZE_LIGHT_COMBINE_IBL
	const float3 irradiance = tx_irrMap.Sample(splr_LE, normal).rgb;
	const float3 diffuseLevel = (1.0f - fresnel) * (1.0f - metalness);
	
	ambient = diffuseLevel * irradiance * albedo;
#	endif
	
#	ifdef _ZE_LIGHT_COMBINE_SSR
	const float3 specularColor = tx_ssr.Sample(splr_PR, tc).rgb;
#	else
	uint width, height, mipCount;
	tx_envMap.GetDimensions(0, width, height, mipCount);
	
	const float3 specularColor = tx_envMap.SampleLevel(splr_LE, normal, roughness * mipCount).rgb;
#	endif
	
	const float2 envBRDF  = tx_brdfLUT.Sample(splr_PE, float2(cosTheta, roughness)).rg;
	ambient += specularColor * (fresnel * envBRDF.x + envBRDF.y);
#else
	float3 ambient = cb_settingsData.AmbientLight;
#endif
	
#ifdef _ZE_LIGHT_COMBINE_AO
	// AO should only affect ambient, GI, and IBL lighting
	const float ssao = tx_ssaoMap[tc * cb_settingsData.RenderSize] / 255.0f;
	ambient *= ssao;
#endif
	return float4(ambient + tx_directLight.Sample(splr_PR, tc).rgb, 1.0f);
}