cbuffer TransformConstatBuffer
{
	matrix transformView;
	matrix transformViewProjection;
};

struct VSOut
{
	float3 viewPos : POSITION;
	float3 viewNormal : NORMAL;
#ifdef _TEX
	float2 tc : TEXCOORD;
#ifdef _TEX_NORMAL
	float3 viewTan : TANGENT;
	float3 viewBitan : BITANGENT;
#endif
#endif
	float4 pos : SV_Position;
};

VSOut main(float3 pos : POSITION, float3 normal : NORMAL
#ifdef _TEX
	, float2 tc : TEXCOORD
#ifdef _TEX_NORMAL
	, float3 tangent : TANGENT,
	float3 bitangent : BITANGENT
#endif
#endif
)
{
	VSOut vso;
	vso.viewPos = (float3) mul(float4(pos, 1.0f), transformView);
	vso.viewNormal = mul(normal, (float3x3) transformView);
#ifdef _TEX
	vso.tc = tc;
#ifdef _TEX_NORMAL
	vso.viewTan = mul(tangent, (float3x3) transformView);
	vso.viewBitan = mul(bitangent, (float3x3) transformView);
#endif
#endif
	vso.pos = mul(float4(pos, 1.0f), transformViewProjection);
	return vso;
}