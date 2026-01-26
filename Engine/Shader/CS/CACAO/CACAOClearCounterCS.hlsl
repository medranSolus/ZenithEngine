#include "Utils/FFX.hlsli"
#include "Buffers.hlsli"

UAV_EX(loadCounter, RWTexture1D<FfxUInt32>, 0, 0);

[numthreads(1, 1, 1)]
void main()
{
	ua_loadCounter[0] = 0;
}