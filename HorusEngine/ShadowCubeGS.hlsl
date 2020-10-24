#include "ViewGB.hlsli"

struct GSIn
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
#ifdef _TEX
	float2 tc : TEXCOORD;
#ifdef _TEX_PAX
	float3 worldBitan : BITANGENT;
#endif
#endif
};

struct GSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
#ifdef _TEX
	float2 tc : TEXCOORD;
#ifdef _TEX_PAX
	float3 worldBitan : BITANGENT;
	float3 cameraDir : CAMERADIR;
#endif
#endif
	uint face : SV_RENDERTARGETARRAYINDEX;
	float4 pos : SV_POSITION;
};

[maxvertexcount(18)]
void main(triangle GSIn input[3], inout TriangleStream<GSOut> output)
{
	for (uint i = 0; i < 6; ++i)
	{
		for (uint j = 0; j < 3; ++j)
		{
			GSOut element;
			element.worldPos = input[j].worldPos;
			element.worldNormal = input[j].worldNormal;
#ifdef _TEX
			element.tc = input[j].tc;
#ifdef _TEX_PAX
			element.worldBitan = input[j].worldBitan;
			element.cameraDir = cb_cameraPos - input[j].worldPos;
#endif
#endif
			element.face = i;
			element.pos = mul(float4(input[j].worldPos, 1.0f), cb_viewProjection[i]);
			output.Append(element);
		}
		output.RestartStrip();
	}
}