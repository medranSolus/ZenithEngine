SamplerState splr : register(s0);
Texture2D tex : register(t0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	return 1.0f - tex.Sample(splr, tc).rgba;
}