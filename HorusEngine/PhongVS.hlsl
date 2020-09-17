#include "UtilsVS.hlsli"
#include "TransformCBuffer.hlsli"

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	//float3 shadowPos : SHADOW_POSITION;
#ifdef _TEX
	float2 tc : TEXCOORD;
#ifdef _TEX_NORMAL
	float3 worldTan : TANGENT;
	float3 worldBitan : BITANGENT;
#endif
#endif
	float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION, float3 normal : NORMAL
#ifdef _TEX
	, float2 tc : TEXCOORD
#ifdef _TEX_NORMAL
	, float3 tangent : TANGENT,
	float3 bitangent : BITANGENT
#endif
#endif
)
{
	VSOut vso;
	vso.worldPos = (float3)mul(float4(pos, 1.0f), cb_transform);
	vso.worldNormal = mul(normal, (float3x3) cb_transform);
	//vso.shadowPos = ToShadowSpacePos(pos, cb_transform, cb_shadowTranslation);
	vso.pos = mul(float4(pos, 1.0f), cb_transformViewProjection);

#ifdef _TEX
	vso.tc = tc;
#ifdef _TEX_NORMAL
	vso.worldTan = mul(tangent, (float3x3) cb_transform);
	vso.worldBitan = mul(bitangent, (float3x3) cb_transform);
#endif
#endif
	return vso;
}