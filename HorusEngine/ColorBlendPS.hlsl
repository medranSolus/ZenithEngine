float4 main(float4 color : COLOR) : SV_TARGET
{
	clip(color.a < 0.005f ? -1 : 1);
	return color;
}