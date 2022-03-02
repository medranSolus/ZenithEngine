#include "CBuffer.hlsli"

struct WorldData
{
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