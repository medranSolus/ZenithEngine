#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "CB/Denoise.hlsli"
#include "Utils/SSAO.hlsli"

UAV2D(aoMapOutput, uint, 0, 2);
TEXTURE_EX(aoMapPrevious, Texture2D<uint>,    0, 3);
TEXTURE_EX(depthEdges,    Texture2D<lpfloat>, 1, 1);

// XeGTAO third+ pass
[numthreads(XE_GTAO_NUMTHREADS_X, XE_GTAO_NUMTHREADS_Y, 1)]
void main(const uint2 dispatchThreadID : SV_DispatchThreadID)
{
	// Computing 2 horizontal pixels at a time (performance optimization)
	const uint2 pixCoordBase = dispatchThreadID * uint2(2, 1);
	XeGTAO_Denoise(pixCoordBase, cb_pbrData.SsaoData, tx_aoMapPrevious, tx_depthEdges, splr_PE, ua_aoMapOutput, ct_denoise.IsLast);
}