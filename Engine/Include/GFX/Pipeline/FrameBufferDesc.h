#pragma once
#include "GFX/Resource/State.h"
#include "GFX/QueueType.h"
#include "FrameResourceDesc.h"
#include "ResourceID.h"
#include "SyncType.h"
#include "TransitionDesc.h"

namespace ZE::GFX::Pipeline
{
	// FrameBuffer creation data
	struct FrameBufferDesc
	{
		std::vector<FrameResourceDesc> ResourceInfo;
		std::vector<std::map<U64, std::pair<Resource::State, QueueType>>> ResourceLifetimes;
		std::vector<std::vector<std::pair<TransitionDesc, QueueType>>> TransitionsPerLevel;
		// Start pass level | Pass level count | Enter sync | Exit sync
		std::vector<std::pair<std::pair<U64, U16>, std::pair<SyncType, SyncType>>> RenderLevels;

		PixelFormat GetFormat(RID id) const noexcept { return ResourceInfo.at(id).Format; }

		void Init(U64 resourceCount, U32 backbufferWidth, U32 backbufferHeight);
		RID AddResource(FrameResourceDesc&& info) noexcept;
		void ComputeWorkflowTransitions(U64 dependencyLevels) noexcept;
	};
}