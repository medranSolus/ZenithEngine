cbuffer TransformConstatBuffer
{
	matrix transformView;
	matrix transformViewProjection;
};

struct VSOut
{
	float3 viewPos : POSITION;
	float3 viewTan : TANGENT;
	float3 viewBitan : BITANGENT;
	float3 viewNormal : NORMAL;
	float2 tc : TEXCOORD;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : POSITION, float3 normal : NORMAL, float2 tc : TEXCOORD, float3 tangent : TANGENT, float3 bitangent : BITANGENT)
{
	VSOut vso;
	vso.viewPos = (float3) mul(float4(pos, 1.0f), transformView);
	vso.viewTan = mul(tangent, (float3x3) transformView);
	vso.viewBitan = mul(bitangent, (float3x3) transformView);
	vso.viewNormal = mul(normal, (float3x3) transformView);
	vso.tc = tc;
	vso.pos = mul(float4(pos, 1.0f), transformViewProjection);
	return vso;
}