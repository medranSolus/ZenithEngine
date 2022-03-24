#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "CB/Denoise.hlsli"
#include "Utils/SSAO.hlsli"

RWTexture2D<uint>  ssaoMapOutput   : register(u0);
Texture2D<uint>    ssaoMapPrevious : register(t0);
Texture2D<lpfloat> depthEdges      : register(t1);

// XeGTAO third+ pass
[numthreads(XE_GTAO_NUMTHREADS_X, XE_GTAO_NUMTHREADS_Y, 1)]
void main(const uint2 dispatchThreadID : SV_DispatchThreadID)
{
	// Computing 2 horizontal pixels at a time (performance optimization)
	const uint2 pixCoordBase = dispatchThreadID * uint2(2, 1);
	XeGTAO_Denoise(pixCoordBase, cb_pbrData.SsaoData, ssaoMapPrevious, depthEdges, splr_PE, ssaoMapOutput, cb_lastDenoise);
}