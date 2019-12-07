cbuffer ConstatBuffer
{
	matrix model;
	matrix modelViewProjection;
};

struct VSOut
{
	float3 worldPos : Position;
	float3 normal : Normal;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 normal : Normal)
{
	VSOut vso;
	vso.worldPos = (float3)mul(model, float4(pos, 1.0f));
    vso.normal = mul((float3x3)model, normal);
	vso.pos = mul(modelViewProjection, float4(pos, 1.0f));
	return vso;
}