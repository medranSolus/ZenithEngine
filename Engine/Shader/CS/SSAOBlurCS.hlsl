#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"

RWTexture2D<float> ssaoMap : register(u0);

static const uint2 THREAD_COUNT = uint2(8, 8);
groupshared float groupSsao[THREAD_COUNT.x][THREAD_COUNT.y];

[numthreads(THREAD_COUNT.x, THREAD_COUNT.y, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID, uint3 threadId : SV_GroupThreadID)
{
	static const int BEGIN = -3;
	static const uint LAST = 3;

	int s = BEGIN;
	uint u = 1;
	uint2 frameBounds = cb_pbrData.FrameDimmensions - 1;
	float ssaoVal = ssaoMap[dispatchId.xy];
	groupSsao[threadId.x][threadId.y] = ssaoVal;

	// Vertical blur
	GroupMemoryBarrierWithGroupSync();
	[unroll]
	for (; s < 0; ++s)
	{
		if ((int)threadId.y + s < 0)
			ssaoVal += ssaoMap[abs((int2)dispatchId.xy + int2(0, s))];
		else
			ssaoVal += groupSsao[threadId.x][threadId.y + s];
	}
	[unroll]
	for (; u <= LAST; ++u)
	{
		const uint offset = threadId.y + u;
		if (offset < THREAD_COUNT.y)
			ssaoVal += groupSsao[threadId.x][offset];
		else if (dispatchId.y + u > frameBounds.y)
			ssaoVal += ssaoMap[dispatchId.xy - uint2(0, u)];
		else
			ssaoVal += ssaoMap[dispatchId.xy + uint2(0, u)];
	}
	ssaoVal /= (LAST * 2 + 1);
	groupSsao[threadId.x][threadId.y] = ssaoVal;

	// Horizontal blur
	GroupMemoryBarrierWithGroupSync();
	[unroll]
	for (s = BEGIN; s < 0; ++s)
	{
		if ((int)threadId.x + s < 0)
			ssaoVal += ssaoMap[abs((int2)dispatchId.xy + int2(s, 0))];
		else
			ssaoVal += groupSsao[threadId.x + s][threadId.y];
	}
	[unroll]
	for (u = 1; u <= LAST; ++u)
	{
		const uint offset = threadId.x + u;
		if (offset < THREAD_COUNT.x)
			ssaoVal += groupSsao[offset][threadId.y];
		else if (dispatchId.x + u > frameBounds.x)
			ssaoVal += ssaoMap[dispatchId.xy - uint2(u, 0)];
		else
			ssaoVal += ssaoMap[dispatchId.xy + uint2(u, 0)];
	}
	ssaoMap[dispatchId.xy] = ssaoVal / (LAST * 2 + 1);

	//groupSsao[threadId.x][threadId.y] = ssaoVal;
	//static const int BEGIN = -3;
	//static const uint LAST = 3;
	//int s = BEGIN;
	//uint u = 1;
	//uint count = 0;
	//float ssaoVal = ssaoMap[dispatchId.xy];

	//// Vertical blur
	//GroupMemoryBarrierWithGroupSync();
	//[unroll]
	//for (; s < 0; ++s)
	//	ssaoVal += (groupSsao[threadId.x][abs((int)threadId.y + s)] - ssaoVal) / ++count;
	//[unroll]
	//for (; u <= LAST; ++u)
	//{
	//	uint offset = threadId.y + u;
	//	if (offset >= THREAD_COUNT.y)
	//		offset = threadId.y - u;
	//	ssaoVal += (groupSsao[threadId.x][offset] - ssaoVal) / ++count;
	//}
	//groupSsao[threadId.x][threadId.y] = ssaoVal;

	//// Horizontal blur
	//GroupMemoryBarrierWithGroupSync();
	//[unroll]
	//for (s = BEGIN; s < 0; ++s)
	//	ssaoVal += (groupSsao[abs((int)threadId.x + s)][threadId.y] - ssaoVal) / ++count;
	//[unroll]
	//for (u = 1; u <= LAST; ++u)
	//{
	//	uint offset = threadId.x + u;
	//	if (offset > THREAD_COUNT.x)
	//		offset = threadId.x - u;
	//	ssaoVal += (groupSsao[offset][threadId.y] - ssaoVal) / ++count;
	//}
	//ssaoMap[dispatchId.xy] = ssaoVal;
}