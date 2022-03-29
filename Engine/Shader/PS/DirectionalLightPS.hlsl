#include "CommonUtils.hlsli"
#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "WorldDataCB.hlsli"
#include "Utils/LightUtils.hlsli"
#include "CB/DirectionalLight.hlsli"

Texture2D shadowMap   : register(t0);
Texture2D normalMap   : register(t1);
Texture2D specularMap : register(t2); // RGB - color, A - power
Texture2D depthMap    : register(t3);

struct PSOut
{
	float4 color    : SV_TARGET0;
	float4 specular : SV_TARGET1;
};

PSOut main(float2 tc : TEXCOORD)
{
	// Position depth reconstruction
	const float3 position = GetWorldPosition(tc, depthMap.Sample(splr_PR, tc).x, cb_worldData.ViewProjectionInverse);
	const float3 shadowColor = DeleteGammaCorr(cb_light.Shadow);
	const float3 lightColor = DeleteGammaCorr(cb_light.Color) * cb_light.Intensity;

	// Shadow test
	const float shadowLevel = 1.0f;
	//const float shadowLevel = GetShadowLevel(normalize(cb_worldData.CameraPos - position), lightDistance,
	//	directionToLight, GetShadowUV(position), splr_AB, shadowMap, cb_pbrData.ShadowMapSize);

	const float3 normal = DecodeNormal(normalMap.Sample(splr_PR, tc).rg);
	const float3 directionToLight = -cb_lightDir;
	const float4 specularData = specularMap.Sample(splr_PR, tc);

	PSOut pso;

	pso.color = float4(lerp(shadowColor, GetDiffuse(lightColor, directionToLight, normal), shadowLevel) * float(shadowLevel >= 0.001f), 0.0f);

	pso.specular = float4(GetSpecular(cb_worldData.CameraPos, directionToLight, position, normal,
		pso.color.rgb * specularData.rgb, specularData.a) * float(shadowLevel > 0.98f), 0.0f);

	return pso;
}