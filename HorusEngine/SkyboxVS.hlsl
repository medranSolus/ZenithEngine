cbuffer TransformCBuffer : register(b0)
{
	matrix cb_viewProjection;
}

struct VSOut
{
	float3 worldPos : POSITION;
	float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION)
{
	VSOut vso;
	vso.worldPos = pos;
	vso.pos = mul(float4(pos, 0.0f), cb_viewProjection); // .w = 0.0f so no translation, only rotation
	vso.pos.z = vso.pos.w; // Depth after perspective divide is 1.0f
	return vso;
}