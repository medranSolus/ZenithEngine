#pragma once
#include "PixelFormat.h"

namespace ZE::GFX::Pipeline
{
	// Additional info for specified buffer
	enum FrameResourceFlags : U8
	{
		None = 0,
		Cube = 1,
		ForceSRV = 2,
		// Cannot be used together with SimultaneousAccess
		ForceDSV = 4,
		// Cannot be used together with ForceDSV
		SimultaneousAccess = 8,
		// When creating SRV over depth stencil choose to use stencil instead of depth
		StencilView = 16
	};

	// Description of single resource in FrameBuffer
	struct FrameResourceDesc
	{
		UInt2 Sizes;
		U16 ArraySize;
		U8 Flags;
		PixelFormat Format;
		ColorF4 ClearColor;
		float ClearDepth = 0.0f;
		U8 ClearStencil = 0;
		U16 MipLevels = 1;
	};
}