#include "Samplers.hlsli"
#include "CBuffer/BlurDirection.hlsli"
#include "../CS/CBuffer/SSAOKernel.hlsli"
#include "SSAOMap.hlsli"

float main(float2 tc : TEXCOORD) : SV_TARGET
{
	float2 delta;
	if (cb_vertical)
		delta = float2(0.0f, 0.125f / cb_noiseTileDimensions.y);
	else
		delta = float2(0.25f / cb_noiseTileDimensions.x, 0.0f);

	static const int RANGE = 3;
	float result = 0.0f;
	[unroll]
	for (int i = -RANGE; i < RANGE; ++i)
		result += tx_ssao.Sample(splr_LR, tc + delta * i).r;
	return result / (RANGE * 2);
}