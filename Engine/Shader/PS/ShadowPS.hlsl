#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "Utils/Utils.hlsli"
#include "CB/Shadow.hlsli"

#ifdef _USE_TEXTURE
TEX2D(tex, 0);
#endif
#ifdef _USE_NORMAL
TEX2D(normalMap, 1);
#endif
#ifdef _USE_PARALLAX
TEX2D(parallax, 3);
#endif

float main(float3 worldPos : POSITION,
	float3 worldNormal : NORMAL,
	float2 tc : TEXCOORD,
	float3 worldTan : TANGENT,
	float3 cameraDir : CAMERADIR) : SV_TARGET
{
	float3 lightToVertex = worldPos - ct_shadow.LightPos;

#ifdef _USE_TEXTURE
	clip(tx_tex.Sample(splr_AR, tc).a - 0.0039f);
#endif

#if defined(_USE_NORMAL) || defined(_USE_PARALLAX)
	const float3x3 TBN = GetTangentToWorld(worldTan, worldNormal);
#ifdef _USE_PARALLAX
	tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), ct_shadow.ParallaxScale, tx_parallax, splr_AR);
	[branch]
	if ((tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f) /*&& useParallax != 0*/)
		discard;
#endif
#endif

#ifdef _USE_NORMAL
	const float3 normal = GetMappedNormal(TBN, tc, tx_normalMap, splr_PR);
	//float depth = 0.0f;
	//if (dot(normal, lightToVertex) <= 0)
	//	depth = GetParallaxDepth(tc, normalize(mul(TBN, -lightToVertex)), tx_parallax, splr_AW);
	lightToVertex -= cb_pbrData.ShadowNormalOffset * normal;
#else
	lightToVertex -= cb_pbrData.ShadowNormalOffset * normalize(worldNormal);
#endif

	return length(lightToVertex) + cb_pbrData.ShadowBias;
}