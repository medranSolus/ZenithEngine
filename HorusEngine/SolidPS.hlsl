cbuffer SolidCBuffer : register(b1)
{
	float3 solidColor;
};

float4 main() : SV_Target
{
	return float4(solidColor, 1.0f);
}