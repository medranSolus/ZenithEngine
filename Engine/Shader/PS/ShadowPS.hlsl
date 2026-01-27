#include "CB/Shadow.hlsli"
#include "CB/PbrFlags.hlsli"
#include "Tex/PbrInputs.hlsli"
#include "Utils/Geometry.hlsli"
#include "Utils/Shadow.hlsli"
#include "Samplers.hlsli"
#include "SettingsDataCB.hlsli"

/* List of permutations:
 *
 * _ZE_TRANSPARENT - texture albedo alpha value may contain cut-out segments
 * _ZE_USE_PARALLAX - use parallax mapping
 */

float main(float3 worldPos : POSITION,
	float3 worldNormal : NORMAL,
	float2 tc : TEXCOORD,
	float4 worldTan : TANGENTPACK,
	float3 cameraDir : CAMERADIR) : SV_TARGET
{
	float3 normal;
#ifndef _ZE_USE_PARALLAX
	if (ct_shadow.Flags & ZE_PBR_USE_NORMAL_TEX)
#endif
	{
		const float3x3 TBN = GetTangentToWorld(worldTan, worldNormal);
#ifdef _ZE_USE_PARALLAX
		tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), ct_shadow.ParallaxScale, tx_height, splr_AR, cb_settingsData.MipBias);
		[branch]
		if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
			discard;
#endif
		normal = GetMappedNormal(TBN, tc, tx_normalMap, splr_AR, cb_settingsData.MipBias);
	}
#ifndef _ZE_USE_PARALLAX
	else
		normal = normalize(worldNormal);
#endif
	
#ifdef _ZE_TRANSPARENT
	clip(tx_albedo.Sample(splr_AR, tc).a - 0.0039f);
#endif
	
	float3 lightToVertex = worldPos - ct_shadow.LightPos;
	//float depth = 0.0f;
	//if (dot(normal, lightToVertex) <= 0)
	//	depth = GetParallaxDepth(tc, normalize(mul(TBN, -lightToVertex)), tx_height, splr_AW, cb_settingsData.MipBias);
	lightToVertex -= cb_settingsData.ShadowNormalOffset * normal;
	return length(lightToVertex) + cb_settingsData.ShadowBias;
}