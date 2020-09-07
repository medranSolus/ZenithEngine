SamplerState splr : register(s0);
TextureCube box : register(t0);

float4 main(float3 worldPos : POSITION) : SV_TARGET
{
	return box.Sample(splr, worldPos);
}