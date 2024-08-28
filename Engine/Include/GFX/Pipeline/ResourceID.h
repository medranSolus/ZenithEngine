#pragma once
#include "Types.h"

namespace ZE
{
	namespace GFX::Pipeline
	{
		// Type for underlaying handles to resources in FrameBuffer
		typedef U16 ResIndex;

		// Handle to single resource in FrameBuffer
		typedef ResIndex ResourceID;
	}

	// Handle to single resource in FrameBuffer
	typedef GFX::Pipeline::ResourceID RID;

	// RID informing that current resource is invalid
	inline constexpr RID INVALID_RID = UINT16_MAX;
	// Backbuffer is always first in resource list
	inline constexpr RID BACKBUFFER_RID = 0;
	// Backbuffer name during resource creation
	inline constexpr std::string_view BACKBUFFER_NAME = "backbuffer";
}