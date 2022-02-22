#include "CBuffer.hlsli"

struct WorldData
{
	matrix ViewProjection;
	matrix ViewProjectionInverse;
	float3 CameraPos;
};
CBUFFER_GLOBAL(worldData, WorldData, 12);