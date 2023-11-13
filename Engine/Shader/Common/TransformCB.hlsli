#ifndef TRANSFORM_CB_HLSLI
#define TRANSFORM_CB_HLSLI
#include "Buffers.hlsli"

// To correctly use this cbuffer, define 'ZE_TRANSFORM_CB_RANGE' to indicate binding range used for constant buffer
CBUFFER(transform, matrix, 0, ZE_TRANSFORM_CB_RANGE);

#endif // TRANSFORM_CB_HLSLI