#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "CB/Denoise.hlsli"
#include "Utils/SSAO.hlsli"

RWTexture2D<uint> ssaoMapOutput : register(u0);
TEXTURE_EX(ssaoMapPrevious, Texture2D<uint>,    0, 0);
TEXTURE_EX(depthEdges,      Texture2D<lpfloat>, 1, 0);

// XeGTAO third+ pass
[numthreads(XE_GTAO_NUMTHREADS_X, XE_GTAO_NUMTHREADS_Y, 1)]
void main(const uint2 dispatchThreadID : SV_DispatchThreadID)
{
	// Computing 2 horizontal pixels at a time (performance optimization)
	const uint2 pixCoordBase = dispatchThreadID * uint2(2, 1);
	XeGTAO_Denoise(pixCoordBase, cb_pbrData.SsaoData, tx_ssaoMapPrevious, tx_depthEdges, splr_PE, ssaoMapOutput, ct_lastDenoise);
}