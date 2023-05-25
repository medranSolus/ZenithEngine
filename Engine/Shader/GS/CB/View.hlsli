#ifndef VIEW_GS_HLSLI
#define VIEW_GS_HLSLI
#include "Buffers.hlsli"

struct ViewBuffer
{
	matrix ViewProjection[6];
};

CBUFFER(view, ViewBuffer, 0);

#endif // VIEW_GS_HLSLI