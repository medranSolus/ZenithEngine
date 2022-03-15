#include "CB/ModelTransform.hlsli"
#include "WorldDataCB.hlsli"

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	float2 tc : TEXCOORD;
	noperspective float4 worldTan : TANGENTPACK;
	float3 cameraDir : CAMERADIR;
	float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION,
	float3 normal : NORMAL,
	float2 tc : TEXCOORD,
	float4 tangent : TANGENTPACK)
{
	VSOut vso;
	vso.worldPos = (float3)mul(float4(pos, 1.0f), cb_transform.Transforms[cb_transformIndex].M);
	vso.worldNormal = mul(normal, (float3x3) cb_transform.Transforms[cb_transformIndex].M);

	vso.tc = tc;
	vso.worldTan = float4(mul(tangent.xyz, (float3x3)cb_transform.Transforms[cb_transformIndex].M), tangent.w);

	vso.cameraDir = (cb_worldData.CameraPos - vso.worldPos) * -1.0f;
	vso.pos = mul(float4(pos, 1.0f), cb_transform.Transforms[cb_transformIndex].MVP);

	return vso;
}