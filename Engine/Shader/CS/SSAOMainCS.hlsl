#include "CommonUtils.hlsli"
#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "WorldDataCB.hlsli"
#include "Utils/SSAO.hlsli"

UAV2D(ssaoMap,    uint,	       0, 2);
UAV2D(depthEdges, unorm float, 1, 3);
TEXTURE_EX(viewDepth,  Texture2D<lpfloat>, 0, 4);
TEXTURE_EX(normalMap,  Texture2D<float2>,  1, 5);
TEXTURE_EX(hilbertLUT, Texture2D<uint>,    2, 6);

// Load encoded normal and convert to viewspace
lpfloat3 LoadNormal(const in uint2 pos)
{
	return (lpfloat3)mul(DecodeNormal(tx_normalMap[pos]), (float3x3)cb_worldData.View);
}

// Screen & temporal noise loader. Without TAA, temporalIndex is always 0
lpfloat2 SpatioTemporalNoise(const in uint2 pixCoord, uniform uint temporalIndex)
{
	// Hilbert curve driving R2 (see https://www.shadertoy.com/view/3tB3z3),
	// why 288? tried out a few and that's the best so far (with XE_HILBERT_LEVEL 6U) - but there's probably better :)
	const uint index = tx_hilbertLUT[pixCoord % 64] + 288 * (temporalIndex % 64);
	// R2 sequence - see http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
	return (lpfloat2)frac(0.5 + index * float2(0.75487766624669276005, 0.5698402909980532659114));
}

// XeGTAO second pass
[numthreads(XE_GTAO_NUMTHREADS_X, XE_GTAO_NUMTHREADS_Y, 1)]
void main(const uint2 pixCoord : SV_DispatchThreadID)
{
	XeGTAO_MainPass(pixCoord,
		cb_pbrData.SsaoSliceCount, cb_pbrData.SsaoStepsPerSlice,
		SpatioTemporalNoise(pixCoord, cb_pbrData.SsaoData.NoiseIndex),
		LoadNormal(pixCoord), cb_pbrData.SsaoData,
		tx_viewDepth, splr_PE, ua_ssaoMap, ua_depthEdges);
}