#pragma once
#include "ResourceAccess.h"
#include "ResourceID.h"
#include "StageSync.h"
#include "TextureLayout.h"

namespace ZE::GFX::Pipeline
{
	// Transition barrier to be performed on texture
	struct BarrierTransition
	{
		RID Resource;
		TextureLayout LayoutBefore;
		TextureLayout LayoutAfter;
		ResourceAccesses AccessBefore;
		ResourceAccesses AccessAfter;
		// What pipeline stages have to complete before running this barrier
		StageSyncs StageBefore;
		// What pipeline stages need to wait before this barrier completes
		StageSyncs StageAfter;
		// Indicate whether it's part of split barrier
		bool IsSplit;
	};
}