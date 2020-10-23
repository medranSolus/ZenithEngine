#include "TransformVB.hlsli"
#ifdef _TEX_PAX
#include "CameraVB.hlsli"
#endif
#include "ShadowVB.hlsli"

struct VSOut
{
	float3 lightToVertex : VECTOR;
#ifdef _TEX
	float2 tc : TEXCOORD;
#ifdef _TEX_PAX
	float3 worldNormal : NORMAL;
	float3 worldBitan : BITANGENT;
	float3 cameraDir : CAMERADIR;
#endif
#endif
	float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION
#ifdef _TEX
	, float3 normal : NORMAL
	, float2 tc : TEXCOORD
#ifdef _TEX_PAX
	, float3 bitangent : BITANGENT
#endif
#endif
)
{
	VSOut vso;
	vso.pos = mul(float4(pos, 1.0f), cb_transformViewProjection);

	const float3 worldPos = mul(float4(pos, 1.0f), cb_transform).xyz;
	vso.lightToVertex = worldPos - cb_lightPos;

#ifdef _TEX
	vso.tc = tc;
#ifdef _TEX_PAX
	vso.worldNormal = mul(normal, (float3x3) cb_transform);
	vso.worldBitan = mul(bitangent, (float3x3) cb_transform);
	vso.cameraDir = cb_cameraPos - worldPos;
#endif
#endif
	return vso;
}