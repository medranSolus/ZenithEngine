SamplerState splr;
Texture2D tex;

static const int r = 7;
static const float accumulatedDivisor = (2 * r + 1) * (2 * r + 1);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	uint width, height;
	tex.GetDimensions(width, height);
	const float dx = 1.0f / width;
	const float dy = 1.0f / height;
	float alphaSum = 0.0f;
	float3 maxColor = 0.0f;
	for (int y = -r; y <= r; ++y)
	{
		for (int x = -r; x <= r; ++x)
		{
			const float4 pixel = tex.Sample(splr, tc + float2(dx * x, dy * y)).rgba;
			alphaSum += pixel.a;
			maxColor = max(pixel.rgb, maxColor);
		}
	}
	return float4(maxColor, alphaSum / accumulatedDivisor);
}