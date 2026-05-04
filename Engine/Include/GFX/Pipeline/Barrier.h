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
		RID Resource = INVALID_RID;
		TextureLayout LayoutBefore = TextureLayout::Undefined;
		TextureLayout LayoutAfter = TextureLayout::Undefined;
		ResourceAccesses AccessBefore = Base(ResourceAccess::None);
		ResourceAccesses AccessAfter = Base(ResourceAccess::None);
		// What pipeline stages have to complete before running this barrier
		StageSyncs StageBefore = Base(StageSync::None);
		// What pipeline stages need to wait before this barrier completes
		StageSyncs StageAfter = Base(StageSync::None);
		BarrierType Type = BarrierType::Immediate;
		U32 Subresource = UINT32_MAX;
	};
}