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

		void ComputeWorkflowTransitions(U64 dependencyLevels) noexcept;
	};
}