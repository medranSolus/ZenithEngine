cbuffer ColorBuffer
{
	float4 faceColors[20];
}

float4 main(uint tID : SV_PrimitiveID) : SV_Target
{
	return faceColors[tID % 20];
}