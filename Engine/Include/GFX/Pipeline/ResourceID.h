#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
	// Handle to single resource in FrameBuffer
	typedef U64 ResourceID;
}
namespace ZE
{
	// Handle to single resource in FrameBuffer
	typedef GFX::Pipeline::ResourceID RID;
}