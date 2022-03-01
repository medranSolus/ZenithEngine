#include "CB/View.hlsli"

struct GSIn
{
	float3 worldPos : POSITION;
};

struct GSOut
{
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
			element.face = i;
			element.pos = mul(float4(input[j].worldPos, 1.0f), cb_view.ViewProjection[i]);
			output.Append(element);
		}
		output.RestartStrip();
	}
}