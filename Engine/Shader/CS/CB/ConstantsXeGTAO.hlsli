#ifndef CONSTANTS_XEGTAO_HLSLI
#define CONSTANTS_XEGTAO_HLSLI
#include "Buffers.hlsli"
#include "Utils/XeGTAO.hlsli"
#include "../Include/XeGTAO.h"

// To correctly use this cbuffer, define 'ZE_XEGTAO_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsXeGTAO
{
	// Should be multiple of 16 B (alignment restrictions)
	GTAOConstants XeGTAOData;
	
	// LOW - 1, MEDIUM - 2, HIGH - 3, ULTRA - 9 (MAX is 9, otherwise change loop unroll value according to "CS/Utils/XeGTAO.hlsli")
	float SliceCount;
	// LOW - 2, MEDIUM - 2, HIGH - 3, ULTRA - 3 (MAX is 3, otherwise change loop unroll value according to "CS/Utils/XeGTAO.hlsli")
	float StepsPerSlice;
};

CBUFFER(xegtaoConsts, ConstantsXeGTAO, 1, ZE_XEGTAO_CB_RANGE);

#endif // CONSTANTS_XEGTAO_HLSLI