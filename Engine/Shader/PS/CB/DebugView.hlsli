#ifndef DEBUG_VIEW_PS_HLSLI
#define DEBUG_VIEW_PS_HLSLI
#include "Buffers.hlsli"

struct Mode
{
	uint ViewMode;
};

CONSTANT(mode, Mode, 0);

static const uint ZE_DEBUG_VIEW_MODE_DEPTH = 1;
static const uint ZE_DEBUG_VIEW_MODE_NORMALS = 2;
static const uint ZE_DEBUG_VIEW_MODE_ALBEDO = 3;
static const uint ZE_DEBUG_VIEW_MODE_METAL = 4;
static const uint ZE_DEBUG_VIEW_MODE_ROUGH = 5;
static const uint ZE_DEBUG_VIEW_MODE_MOTION = 6;
static const uint ZE_DEBUG_VIEW_MODE_REACTIVE = 7;
static const uint ZE_DEBUG_VIEW_MODE_DIRECT_LIGHT = 8;
static const uint ZE_DEBUG_VIEW_MODE_SSAO = 9;
static const uint ZE_DEBUG_VIEW_MODE_SSR = 10;
static const uint ZE_DEBUG_VIEW_MODE_RENDERED_SCENE = 11;
static const uint ZE_DEBUG_VIEW_MODE_UPSCALED_SCENE = 12;
static const uint ZE_DEBUG_VIEW_MODE_OUTLINE = 13;

#endif // DEBUG_VIEW_PS_HLSLI