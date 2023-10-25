#ifndef WORLD_DATA_CB_HLSLI
#define WORLD_DATA_CB_HLSLI
#include "Buffers.hlsli"

struct WorldData
{
	matrix View;
	matrix ViewProjection;
	matrix PrevViewProjection;
	matrix ViewProjectionInverse;
	matrix PrevViewProjectionInverse;
	float3 CameraPos;
	float NearClip;
	float2 Jitter;
	float2 PrevJitter;
};
CBUFFER_GLOBAL(worldData, WorldData, 12, 1);

// Convert depth value from logarithmic depth space to linear view space
float GetLinearDepth(const in float depth)
{
	return cb_worldData.NearClip / depth;
}

#endif // WORLD_DATA_CB_HLSLI