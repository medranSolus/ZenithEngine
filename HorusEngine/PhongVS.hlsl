cbuffer TransformConstatBuffer
{
	matrix modelView;
	matrix modelViewProjection;
};

struct VSOut
{
    float3 cameraPos : POSITION;
	float3 normal : NORMAL;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : POSITION, float3 normal : NORMAL)
{
	VSOut vso;
    vso.cameraPos = (float3) mul(modelView, float4(pos, 1.0f));
    vso.normal = mul((float3x3)modelView, normal);
	vso.pos = mul(modelViewProjection, float4(pos, 1.0f));
	return vso;
}