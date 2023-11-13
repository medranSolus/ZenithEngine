#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "Utils/Utils.hlsli"
#include "CB/Shadow.hlsli"

#ifdef _ZE_USE_TEXTURE
TEX2D(tex, 0, 2);
#endif
#ifdef _ZE_USE_NORMAL
TEX2D(normalMap, 1, 2);
#endif
#ifdef _ZE_USE_PARALLAX
TEX2D(parallax, 3, 2);
#endif

float main(float3 worldPos : POSITION,
	float3 worldNormal : NORMAL,
	float2 tc : TEXCOORD,
	float3 worldTan : TANGENT,
	float3 cameraDir : CAMERADIR) : SV_TARGET
{
	float3 lightToVertex = worldPos - ct_shadow.LightPos;

#ifdef _ZE_USE_TEXTURE
	clip(tx_tex.Sample(splr_AR, tc).a - 0.0039f);
#endif

#if defined(_ZE_USE_NORMAL) || defined(_ZE_USE_PARALLAX)
	const float3x3 TBN = GetTangentToWorld(worldTan, worldNormal);
#	ifdef _ZE_USE_PARALLAX
	tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), ct_shadow.ParallaxScale, tx_parallax, splr_AR, cb_pbrData.MipBias);
	[branch]
	if ((tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f) /*&& useParallax != 0*/)
		discard;
#	endif
#endif

#ifdef _ZE_USE_NORMAL
	const float3 normal = GetMappedNormal(TBN, tc, tx_normalMap, splr_PR, cb_pbrData.MipBias);
	//float depth = 0.0f;
	//if (dot(normal, lightToVertex) <= 0)
	//	depth = GetParallaxDepth(tc, normalize(mul(TBN, -lightToVertex)), tx_parallax, splr_AW, cb_pbrData.MipBias);
	lightToVertex -= cb_pbrData.ShadowNormalOffset * normal;
#else
	lightToVertex -= cb_pbrData.ShadowNormalOffset * normalize(worldNormal);
#endif

	return length(lightToVertex) + cb_pbrData.ShadowBias;
}