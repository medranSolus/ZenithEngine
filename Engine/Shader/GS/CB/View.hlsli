#include "CBuffer.hlsli"

struct ViewBuffer
{
	matrix ViewProjection[6];
};

CBUFFER(view, ViewBuffer, 0);