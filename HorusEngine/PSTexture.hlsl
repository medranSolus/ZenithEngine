Texture2D tex;
SamplerState splr;

float4 main(float2 texc : TexCoord) : SV_Target
{
	return tex.Sample(splr, texc);
}