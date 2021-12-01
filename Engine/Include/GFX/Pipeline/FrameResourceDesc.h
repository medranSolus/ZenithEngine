#pragma once
#include "PixelFormat.h"

namespace ZE::GFX::Pipeline
{
	// Additional info for specified buffer
	enum FrameResourceFlags : U8 { None, Cube, ForceSRV };

	// Description of single resource in FrameBuffer
	struct FrameResourceDesc
	{
		U32 Width;
		U32 Height;
		U16 ArraySize;
		U8 Flags;
		PixelFormat Format;
		ColorF4 ClearColor;
		float ClearDepth = 0.0f;
		U8 ClearStencil = 0;
	};
}