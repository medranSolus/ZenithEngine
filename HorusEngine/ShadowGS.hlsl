#include "ViewGB.hlsli"

struct GSIn
{
	float3 lightToVertex : VECTOR;
#ifdef _TEX
	float2 tc : TEXCOORD;
#endif
	float3 worldPos : POSITION;
};

struct GSOut
{
	float3 lightToVertex : VECTOR;
#ifdef _TEX
	float2 tc : TEXCOORD;
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
			element.lightToVertex = input[j].lightToVertex;
#ifdef _TEX
			element.tc = input[j].tc;
#endif
			element.face = i;
			element.pos = mul(float4(input[j].worldPos, 1.0f), cb_viewProjection[i]);
			output.Append(element);
		}
		output.RestartStrip();
	}
}