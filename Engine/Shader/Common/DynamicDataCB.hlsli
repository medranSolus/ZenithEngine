#ifndef DYNAMIC_DATA_CB_HLSLI
#define DYNAMIC_DATA_CB_HLSLI
#include "Buffers.hlsli"

struct DynamicData
{
	matrix View;
	matrix ViewProjection;
	matrix ViewProjectionInverse;
	float3 CameraPos;
	float NearClip;
	float2 JitterCurrent;
	float2 JitterPrev;
};
CBUFFER_GLOBAL(dynamicData, DynamicData, 12, 1);

// Convert depth value from logarithmic depth space to linear view space
float GetLinearDepth(const in float depth)
{
	return cb_dynamicData.NearClip / depth;
}

#endif // DYNAMIC_DATA_CB_HLSLI