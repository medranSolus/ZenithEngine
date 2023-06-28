#ifndef MODEL_TRANSFORM_VS_HLSLI
#define MODEL_TRANSFORM_VS_HLSLI
#include "Buffers.hlsli"

struct ModelTransform
{
	matrix M;
	matrix MVP;
};

CBUFFER(transform, ModelTransform, 0, 3);

#endif // MODEL_TRANSFORM_VS_HLSLI