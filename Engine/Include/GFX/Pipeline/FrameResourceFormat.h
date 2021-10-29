#pragma once
#include "PixelFormat.h"

namespace ZE::GFX::Pipeline
{
	// Format and clear value of frame resource
	struct FrameResourceFormat
	{
		PixelFormat Format;
		ColorF4 ClearColor;
		float ClearDepth = 0.0f;
		U8 ClearStencil = 0;
	};
}