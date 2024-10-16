#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
	// Handle to single resource in FrameBuffer
	typedef U16 ResourceID;
}
namespace ZE
{
	// Handle to single resource in FrameBuffer
	typedef GFX::Pipeline::ResourceID RID;
	// RID informing that current resource is invalid
	inline constexpr RID INVALID_RID = UINT16_MAX;
}