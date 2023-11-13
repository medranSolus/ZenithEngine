#include "CB/ModelTransform.hlsli"
#include "WorldDataCB.hlsli"

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	float2 tc : TEXCOORD;
	float3 worldTan : TANGENT;
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
	float3 tangent : TANGENT)
{
	VSOut vso;
	vso.worldPos = (float3)mul(float4(pos, 1.0f), cb_transform.M);
	vso.worldNormal = mul(normal, (float3x3) cb_transform.M);

	vso.tc = tc;
	vso.worldTan = mul(tangent, (float3x3)cb_transform.M);

	vso.cameraDir = vso.worldPos - cb_worldData.CameraPos;
	vso.pos = mul(float4(pos, 1.0f), cb_transform.MVP);
#ifdef _ZE_OUTPUT_MOTION
	vso.prevPos = mul(float4(pos, 1.0f), cb_transform.PrevMVP);
	vso.currentPos = vso.pos;
#endif

	return vso;
}