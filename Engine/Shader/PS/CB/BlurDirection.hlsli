#ifndef BLUR_DIRECTION_PS_HLSLI
#define BLUR_DIRECTION_PS_HLSLI
#include "Buffers.hlsli"

struct BlurDirection
{
	bool IsVertical;
};

CONSTANT(blurDir, BlurDirection, 0);

#endif // BLUR_DIRECTION_PS_HLSLI