cbuffer TransformConstatBuffer
{
	matrix transform;
	matrix scaling;
	matrix view;
	matrix projection;
};

struct VSOut
{
	float3 cameraPos : POSITION;
	float3 normal : NORMAL;
	float2 tc : TEXCOORD;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : POSITION, float3 normal : NORMAL, float2 tc : TEXCOORD)
{
	VSOut vso;
	matrix viewTransform = mul(transform, view);
	matrix viewTransformScaling = mul(scaling, viewTransform);
	vso.cameraPos = (float3) mul(float4(pos, 1.0f), viewTransformScaling);
	vso.normal = mul(normal, (float3x3) viewTransform);
	vso.tc = tc;
	vso.pos = mul(float4(pos, 1.0f), mul(viewTransformScaling, projection));
	return vso;
}