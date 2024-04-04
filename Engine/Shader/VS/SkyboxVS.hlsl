#include "DynamicDataCB.hlsli"

struct VSOut
{
	float3 worldPos : POSITION;
	float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION)
{
	VSOut vso;
	vso.worldPos = pos;
	vso.pos = mul(float4(pos, 0.0f), cb_dynamicData.ViewProjection); // .w = 0.0f so no translation, only rotation
	vso.pos.z = 0.0f; // Depth as zero so far plane in reverse depth buffer
	return vso;
}