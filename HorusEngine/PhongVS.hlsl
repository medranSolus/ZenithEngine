#include "UtilsVS.hlsli"
#include "TransformCBuffer.hlsli"
#include "ShadowCBuffer.hlsli"

struct VSOut
{
	float3 viewPos : POSITION;
	float3 viewNormal : NORMAL;
	float4 shadowPos : SHADOW_POSITION;
#ifdef _TEX
	float2 tc : TEXCOORD;
#ifdef _TEX_NORMAL
	float3 viewTan : TANGENT;
	float3 viewBitan : BITANGENT;
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
	vso.viewPos = (float3) mul(float4(pos, 1.0f), cb_transformView);
	vso.viewNormal = mul(normal, (float3x3) cb_transformView);
	vso.shadowPos = ToShadowSpacePos(pos, cb_transform, cb_shadowViewProjection);
	vso.pos = mul(float4(pos, 1.0f), cb_transformViewProjection);

#ifdef _TEX
	vso.tc = tc;
#ifdef _TEX_NORMAL
	vso.viewTan = mul(tangent, (float3x3) cb_transformView);
	vso.viewBitan = mul(bitangent, (float3x3) cb_transformView);
#endif
#endif
	return vso;
}