#include "CB/Transform.hlsli"

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	float2 tc : TEXCOORD;
	float3 worldBitan : BITANGENT;
	float3 cameraDir : CAMERADIR;
	float4 pos : SV_POSITION;
};

VSOut main(/*float3 pos : POSITION,
	float3 normal : NORMAL,
	float2 tc : TEXCOORD,
	float3 bitangent : BITANGENT*/)
{
	float3 pos = 0.0f;
	float3 normal = 0.0f;
	float2 tc = 0.0f;
	float3 bitangent = 0.0f;

	VSOut vso;
	vso.worldPos = (float3)mul(float4(pos, 1.0f), cb_transform.Transforms[cb_transformIndex].M);
	vso.worldNormal = mul(normal, (float3x3) cb_transform.Transforms[cb_transformIndex].M);

	vso.tc = tc;
	vso.worldBitan = mul(bitangent, (float3x3) cb_transform.Transforms[cb_transformIndex].M);

	vso.cameraDir = cb_transform.CameraPos - vso.worldPos;
	vso.cameraDir.y *= -1.0f;
	vso.pos = mul(float4(pos, 1.0f), cb_transform.Transforms[cb_transformIndex].MVP);

	return vso;
}