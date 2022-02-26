#include "CBuffer.hlsli"

struct TransformArray
{
	// Size according to 64KB / sizeof(matrix)
	matrix Transforms[1024];
};

CBUFFER(transformIndex, uint, 0);
CBUFFER(transform, TransformArray, 1);