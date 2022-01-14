#pragma once
#include "GFX/Resource/State.h"
#include "GFX/QueueType.h"
#include "FrameResourceDesc.h"
#include "ResourceID.h"
#include "TransitionDesc.h"

namespace ZE::GFX::Pipeline
{
	// FrameBuffer creation data
	struct FrameBufferDesc
	{
		std::vector<FrameResourceDesc> ResourceInfo;
		std::vector<std::map<RID, Resource::State>> ResourceLifetimes;
		std::vector<std::vector<TransitionDesc>> TransitionsPerLevel;

		PixelFormat GetFormat(RID id) const noexcept { return ResourceInfo.at(id).Format; }

		void Init(U64 resourceCount, U32 backbufferWidth, U32 backbufferHeight);
		RID AddResource(FrameResourceDesc&& info) noexcept;
		void ComputeWorkflowTransitions(U64 dependencyLevels) noexcept;
	};
}