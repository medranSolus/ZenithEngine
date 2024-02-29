#include "CB/ModelTransform.hlsli"
#include "WorldDataCB.hlsli"

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	float2 tc : TEXCOORD;
	float4 worldTan : TANGENTPACK;
	float3 cameraDir : CAMERADIR;
#ifdef _ZE_OUTPUT_MOTION
	float4 prevPos : PREVPOSITION;
	float4 currentPos : CURRPOSITION;
#endif
	float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION,
	float3 normal : NORMAL,
	float2 tc : TEXCOORD,
	float4 tangent : TANGENTPACK)
{
	VSOut vso;
	vso.worldPos = (float3)mul(float4(pos, 1.0f), cb_transform.M);
	vso.worldNormal = mul(normal, (float3x3) cb_transform.M);

	vso.tc = tc;
	vso.worldTan = float4(mul(tangent.xyz, (float3x3) cb_transform.M), tangent.w);

	vso.cameraDir = vso.worldPos - cb_worldData.CameraPos;
	vso.pos = mul(float4(pos, 1.0f), cb_transform.MVP);
#ifdef _ZE_OUTPUT_MOTION
	vso.prevPos = mul(float4(pos, 1.0f), cb_transform.PrevMVP);
	vso.currentPos = vso.pos;
#endif

	return vso;
}