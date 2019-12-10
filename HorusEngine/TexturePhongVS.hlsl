cbuffer TransformConstatBuffer
{
	matrix modelView;
	matrix modelViewProjection;
};

struct VSOut
{
    float3 cameraPos : Position;
	float3 normal : Normal;
    float2 tc : TexCoord;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 normal : Normal, float2 tc : TexCoord)
{
	VSOut vso;
    vso.cameraPos = (float3) mul(modelView, float4(pos, 1.0f));
    vso.normal = mul((float3x3)modelView, normal);
    vso.tc = tc;
	vso.pos = mul(modelViewProjection, float4(pos, 1.0f));
	return vso;
}