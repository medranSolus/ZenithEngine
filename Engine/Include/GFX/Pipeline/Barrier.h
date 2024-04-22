#pragma once
#include "ResourceAccess.h"
#include "ResourceID.h"
#include "StageSync.h"
#include "TextureLayout.h"

namespace ZE::GFX::Pipeline
{
	// Type of barrier to be performed, allowing for split barrier
	enum class BarrierType : U8 { Immediate, SplitBegin, SplitEnd };

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
		BarrierType Type = BarrierType::Immediate;
	};
}