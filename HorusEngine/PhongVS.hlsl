#include "TransformCBuffer.fx"
#include "ShadowCBuffer.fx"

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
	float4 pos : SV_Position;
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
	vso.viewPos = (float3) mul(float4(pos, 1.0f), transformView);
	vso.viewNormal = mul(normal, (float3x3) transformView);

	const float4 shadowSpacePos = mul(mul(float4(pos, 1.0f), transform), shadowViewProjection);
	vso.shadowPos = shadowSpacePos * float4(0.5f, -0.5f, 1.0f, 1.0f) + float4(0.5f, 0.5f, 0.0f, 0.0f) * shadowSpacePos.w;

#ifdef _TEX
	vso.tc = tc;
#ifdef _TEX_NORMAL
	vso.viewTan = mul(tangent, (float3x3) transformView);
	vso.viewBitan = mul(bitangent, (float3x3) transformView);
#endif
#endif
	vso.pos = mul(float4(pos, 1.0f), transformViewProjection);
	return vso;
}