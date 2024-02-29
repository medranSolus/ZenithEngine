#include "WorldDataCB.hlsli"
#include "CB/View.hlsli"

struct GSIn
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	float2 tc : TEXCOORD;
	float4 worldTan : TANGENTPACK;
};

struct GSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	float2 tc : TEXCOORD;
	float4 worldTan : TANGENTPACK;
	float3 cameraDir : CAMERADIR;
	uint face : SV_RENDERTARGETARRAYINDEX;
	float4 pos : SV_POSITION;
};

[maxvertexcount(18)]
void main(triangle GSIn input[3], inout TriangleStream<GSOut> output)
{
	[unroll(6)]
	for (uint i = 0; i < 6; ++i)
	{
		[unroll(3)]
		for (uint j = 0; j < 3; ++j)
		{
			GSOut element;
			element.worldPos = input[j].worldPos;
			element.worldNormal = input[j].worldNormal;
			element.tc = input[j].tc;
			element.worldTan = input[j].worldTan;
			element.cameraDir = cb_worldData.CameraPos - input[j].worldPos;
			element.face = i;
			element.pos = mul(float4(input[j].worldPos, 1.0f), cb_view.ViewProjection[i]);
			output.Append(element);
		}
		output.RestartStrip();
	}
}