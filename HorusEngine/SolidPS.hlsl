cbuffer SolidCBuffer : register(b1)
{
	float3 cb_solidColor;
};

float4 main() : SV_TARGET
{
	return float4(cb_solidColor, 1.0f);
}