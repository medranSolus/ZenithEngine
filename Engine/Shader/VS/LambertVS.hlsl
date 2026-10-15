#define ZE_BBQ_CB_RANGE 4
#include "CB/ModelGeometryIO.hlsli"
#include "CB/ModelTransform.hlsli"
#include "DynamicDataCB.hlsli"

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

VSOut main(ModelVSIn vsi)
{
	float3 pos = GetPosition(vsi.PosPack);
	float3 normal = GetNormal(vsi);
	
	VSOut vso;
	vso.worldPos = (float3)mul(float4(pos, 1.0f), cb_transform.M);
	vso.worldNormal = mul(normal, (float3x3) cb_transform.M);

	vso.tc = vsi.UV;
	vso.worldTan = float4(mul(GetTangent(vsi, normal), (float3x3) cb_transform.M), GetBitanHandedness(vsi));

	vso.cameraDir = vso.worldPos - cb_dynamicData.CameraPos;
	vso.pos = mul(float4(pos, 1.0f), cb_transform.MVP);
#ifdef _ZE_OUTPUT_MOTION
	vso.prevPos = mul(float4(pos, 1.0f), cb_transform.PrevMVP);
	vso.currentPos = vso.pos;
#endif

	return vso;
}