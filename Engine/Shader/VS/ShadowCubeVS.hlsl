#define ZE_BBQ_CB_RANGE 4
#include "CB/ModelGeometryIO.hlsli"
#define ZE_TRANSFORM_CB_RANGE 5
#include "TransformCB.hlsli"

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	float2 tc : TEXCOORD;
	float4 worldTan : TANGENTPACK;
};

VSOut main(ModelVSIn vsi)
{
	float3 pos = GetPosition(vsi.PosPack);
	float3 normal = GetNormal(vsi);
	
	VSOut vso;
	vso.worldPos = mul(float4(pos, 1.0f), cb_transform).xyz;
	vso.worldNormal = mul(normal, (float3x3) cb_transform);

	vso.tc = vsi.UV;
	vso.worldTan = float4(mul(GetTangent(vsi, normal), (float3x3) cb_transform), GetBitanHandedness(vsi));

	return vso;
}