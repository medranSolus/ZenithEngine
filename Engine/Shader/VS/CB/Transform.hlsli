#include "CBuffer.hlsli"

struct ModelTransform
{
	matrix M;
	matrix MVP;
};

struct TransformArray
{
	float3 CameraPos;
	// Size according to 64KB / sizeof(ModelTransform)
	ModelTransform Transforms[511];
};

CBUFFER(transformIndex, uint, 0);
CBUFFER(transform, TransformArray, 1);