#ifndef SSAO_CS_HLSLI
#define SSAO_CS_HLSLI

#define VA_SATURATE saturate
#define XE_GTAO_USE_HALF_FLOAT_PRECISION 0
#define XE_GTAO_FP32_DEPTHS
/* Things modified in the source:
*
* Inclusion of library header for proper path: XeGTAO.h -> ../Include/XeGTAO.h
* Removed component accesses on literal constants: 1.xxx -> 1
* Specified max unroll number for loop with stepsPerSlice (line 417): [unroll] -> [unroll(3)]
* Specified unroll for loop with sliceCount (line 370): //[unroll] -> [unroll(9)]
* For DX11 in XeGTAO_Denoise() replaced GatherRed with texture fetches
*/
#include "XeGTAO.hlsli"

#endif // SSAO_CS_HLSLI