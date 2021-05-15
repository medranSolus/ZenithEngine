#include "CBuffer/TransformVB.hlsli"
#ifdef _TEX_PAX
#include "CBuffer/CameraVB.hlsli"
#endif

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
#ifdef _TEX
	float2 tc : TEXCOORD;
#ifdef _TEX_PAX
	float3 worldBitan : BITANGENT;
	float3 cameraDir : CAMERADIR;
#endif
#endif
	float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION, float3 normal : NORMAL
#ifdef _TEX
	, float2 tc : TEXCOORD
#ifdef _TEX_PAX
	, float3 bitangent : BITANGENT
#endif
#endif
)
{
	VSOut vso;
	vso.pos = mul(float4(pos, 1.0f), cb_transformViewProjection);
	vso.worldPos = mul(float4(pos, 1.0f), cb_transform).xyz;
	vso.worldNormal = mul(normal, (float3x3) cb_transform);

#ifdef _TEX
	vso.tc = tc;
#ifdef _TEX_PAX
	vso.worldBitan = mul(bitangent, (float3x3) cb_transform);
	vso.cameraDir = cb_cameraPos - vso.worldPos;
#endif
#endif
	return vso;
}