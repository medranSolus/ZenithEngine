#include "CBuffer.hlsli"

struct ModelTransform
{
	matrix M;
	matrix MVP;
};

struct TransformArray
{
	// Size according to 64KB / sizeof(ModelTransform)
	ModelTransform Transforms[512];
};

CBUFFER(transformIndex, uint, 0);
CBUFFER(transform, TransformArray, 1);