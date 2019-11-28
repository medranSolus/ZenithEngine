cbuffer ColorBuffer
{
	float4 faceColors[6];
}

float4 main(uint tID : SV_PrimitiveID) : SV_Target
{
	return faceColors[tID / 2 % 6];
}