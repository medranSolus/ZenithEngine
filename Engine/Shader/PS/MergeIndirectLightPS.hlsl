#include "GBufferUtils.hlsli"
#include "Samplers.hlsli"
#include "DynamicDataCB.hlsli"
#include "Utils/LightUtils.hlsli"

TEX2D(ssr, 0, 0);
TEX2D(brdfLUT, 1, 1);
TEX2D(depthMap, 2, 2);
TEXTURE_EX(normalMap,      Texture2D<CodedNormalGB>,    3, 2);
TEXTURE_EX(albedo,         Texture2D<AlbedoGB>,         4, 2);
TEXTURE_EX(materialParams, Texture2D<PackedMaterialGB>, 5, 2);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float3 position = GetWorldPosition(tc, tx_depthMap.Sample(splr_PR, tc).x, cb_dynamicData.ViewProjectionInverse);
	const float3 normal = DecodeNormal(tx_normalMap.Sample(splr_PR, tc));
	const float3 albedo = tx_albedo.Sample(splr_PR, tc).rgb;
	const PackedMaterialGB materialData = tx_materialParams.Sample(splr_PR, tc);
	
	const float3 directionToCamera = normalize(cb_dynamicData.CameraPos - position);
	const float cosTheta = max(dot(normal, directionToCamera), 0.0f);
	const float2 envBRDF = tx_brdfLUT.Sample(splr_PE, float2(cosTheta, GetRoughness(materialData))).rg;
	
	const float3 radiance = tx_ssr.Sample(splr_PR, tc).rgb;
	return float4(radiance * (lerp(0.04f, albedo, GetMetalness(materialData)) * envBRDF.x + envBRDF.y), 0.0f);
}