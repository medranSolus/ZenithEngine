#include "CBuffer.hlsli"

struct ViewBuffer
{
	matrix ViewProjection[6];
};

struct ViewArray
{
	// Size according to 64KB / sizeof(ViewBuffer)
	ViewBuffer Cube[170];
};

CBUFFER(viewIndex, uint, 0);
CBUFFER(view, ViewArray, 1);