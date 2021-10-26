#pragma once
#include "PixelFormat.h"

namespace ZE::GFX::Pipeline
{
	// Description of single resource in FrameBuffer
	struct FrameResourceDesc
	{
		U32 Width;
		U32 Height;
		U16 ArraySize;
		std::vector<PixelFormat> Formats;
	};
}