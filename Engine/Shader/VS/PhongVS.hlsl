#include "CBuffer/Transform.hlsli"
#ifdef _TEX_PAX
#include "CBuffer/Camera.hlsli"
#endif

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
#ifdef _TEX
	float2 tc : TEXCOORD;
#ifdef _TEX_NORMAL
	float3 worldBitan : BITANGENT;
#ifdef _TEX_PAX
	float3 cameraDir : CAMERADIR;
#endif
#endif
#endif
	float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION, float3 normal : NORMAL
#ifdef _TEX
	, float2 tc : TEXCOORD
#ifdef _TEX_NORMAL
	, float3 bitangent : BITANGENT
#endif
#endif
)
{
	VSOut vso;
	vso.worldPos = (float3)mul(float4(pos, 1.0f), cb_transform);
	vso.worldNormal = mul(normal, (float3x3) cb_transform);
	vso.pos = mul(float4(pos, 1.0f), cb_transformViewProjection);

#ifdef _TEX
	vso.tc = tc;
#ifdef _TEX_NORMAL
	vso.worldBitan = mul(bitangent, (float3x3) cb_transform);
#ifdef _TEX_PAX
	vso.cameraDir = cb_cameraPos - vso.worldPos;
	vso.cameraDir.y *= -1.0f;
#endif
#endif
#endif
	return vso;
}