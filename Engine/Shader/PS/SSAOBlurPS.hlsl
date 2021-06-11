#include "Utils/Samplers.hlsli"
#include "CBuffer/BlurDirection.hlsli"
#include "CBuffer/SSAOKernel.hlsli"

Texture2D ssaoMap : register(t11);

float main(float2 tc : TEXCOORD) : SV_TARGET
{
	float2 delta;
	if (cb_vertical)
		delta = float2(0.0f, 0.125f / cb_tileDimensions.y);
	else
		delta = float2(0.25f / cb_tileDimensions.x, 0.0f);

	static const int RANGE = 3;
	float result = 0.0f;
	[unroll]
	for (int i = -RANGE; i < RANGE; ++i)
		result += ssaoMap.Sample(splr_LR, tc + delta * i).r;
	return result / (RANGE * 2);
}