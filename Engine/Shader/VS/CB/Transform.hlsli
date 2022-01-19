#include "CBuffer.hlsli"

struct ModelTransform
{
	matrix M;
	matrix MVP;
};
CBUFFER(transform, ModelTransform, 0);