#pragma once
#include "FrameResourceFormat.h"

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
		std::vector<FrameResourceFormat> Formats;
	};
}