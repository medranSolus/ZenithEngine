#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "Utils/Utils.hlsli"
#include "CB/Shadow.hlsli"

Texture2D tex : register(t0);
Texture2D normalMap : register(t1);
Texture2D parallax : register(t3);

float main(float3 worldPos : POSITION,
	float3 worldNormal : NORMAL,
	float2 tc : TEXCOORD,
	float3 worldTan : TANGENT,
	float3 cameraDir : CAMERADIR) : SV_TARGET
{
	float3 lightToVertex = worldPos - cb_lightPos;

	if (cb_materialFlags & FLAG_USE_TEXTURE)
		clip(tex.Sample(splr_AR, tc).a - 0.0039f);

	if (cb_materialFlags & (FLAG_USE_NORMAL | FLAG_USE_PARALLAX))
	{
		const float3x3 TBN = GetTangentToWorld(worldTan, worldNormal);
		[branch]
		if (cb_materialFlags & FLAG_USE_PARALLAX)
		{
			tc = GetParallaxMapping(tc, normalize(mul(TBN, cameraDir)), cb_parallaxScale, parallax, splr_AR);
			if (tc.x > 1.0f || tc.y > 1.0f || tc.x < 0.0f || tc.y < 0.0f)
				discard;
		}

		const float3 normal = GetMappedNormal(TBN, tc, normalMap, splr_PR);
		//float depth = 0.0f;
		//if (dot(normal, lightToVertex) <= 0)
		//	depth = GetParallaxDepth(tc, normalize(mul(TBN, -lightToVertex)), parallax, splr_AW);
		lightToVertex -= normal * (/*depth + */cb_pbrData.ShadowNormalOffset);
	}
	else
		lightToVertex -= cb_pbrData.ShadowNormalOffset * normalize(worldNormal);

	return length(lightToVertex) + cb_pbrData.ShadowBias;
}