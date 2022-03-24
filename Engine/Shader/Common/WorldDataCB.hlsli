#ifndef WORLD_DATA_CB_HLSLI
#define WORLD_DATA_CB_HLSLI
#include "CBuffer.hlsli"

struct WorldData
{
	matrix View;
	matrix ViewProjection;
	matrix ViewProjectionInverse;
	float3 CameraPos;
	float NearClip;
	float FarClip;
};
CBUFFER_GLOBAL(worldData, WorldData, 12);

// Convert depth value from logarithmic depth space to linear view space
float GetLinearDepth(const in float depth)
{
	return cb_worldData.NearClip * cb_worldData.FarClip / (cb_worldData.FarClip + depth * (cb_worldData.NearClip - cb_worldData.FarClip));
}

#endif // WORLD_DATA_CB_HLSLI