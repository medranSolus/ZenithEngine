#include "GBufferUtils.hlsli"
#include "Samplers.hlsli"
#include "SettingsDataCB.hlsli"
#include "DynamicDataCB.hlsli"
#define ZE_XEGTAO_CB_RANGE 7
#include "CB/ConstantsXeGTAO.hlsli"

UAV2D(ssaoMap,    uint,	       0, 2);
UAV2D(depthEdges, unorm float, 1, 3);
TEXTURE_EX(viewDepth,  Texture2D<lpfloat>,       0, 4);
TEXTURE_EX(normalMap,  Texture2D<CodedNormalGB>, 1, 5);
TEXTURE_EX(hilbertLUT, Texture2D<uint>,          2, 6);

// Load encoded normal and convert to viewspace
lpfloat3 LoadNormal(const in uint2 pos)
{
	return (lpfloat3)mul(DecodeNormal(tx_normalMap[pos]), (float3x3)cb_dynamicData.View);
}

// Screen & temporal noise loader. Without TAA, temporalIndex is always 0
lpfloat2 SpatioTemporalNoise(const in uint2 pixCoord, uniform uint temporalIndex)
{
	// Hilbert curve driving R2 (see https://www.shadertoy.com/view/3tB3z3),
	// why 288? tried out a few and that's the best so far (with XE_HILBERT_LEVEL 6U) - but there's probably better :)
	const uint index = tx_hilbertLUT[pixCoord % 64] + 288 * temporalIndex;
	// R2 sequence - see http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
	return (lpfloat2)frac(0.5 + index * float2(0.75487766624669276005, 0.5698402909980532659114));
}

// XeGTAO second pass
[numthreads(XE_GTAO_NUMTHREADS_X, XE_GTAO_NUMTHREADS_Y, 1)]
void main(const uint2 pixCoord : SV_DispatchThreadID)
{
	XeGTAO_MainPass(pixCoord,
		cb_xegtaoConsts.SliceCount, cb_xegtaoConsts.StepsPerSlice,
		SpatioTemporalNoise(pixCoord, cb_xegtaoConsts.XeGTAOData.NoiseIndex),
		LoadNormal(pixCoord), cb_xegtaoConsts.XeGTAOData,
		tx_viewDepth, splr_PE, ua_ssaoMap, ua_depthEdges);
}