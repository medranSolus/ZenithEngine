#include "GFX/Pipeline/FrameBufferDesc.h"
#include "Settings.h"

namespace ZE::GFX::Pipeline
{
	void FrameBufferDesc::Init(U64 resourceCount, U32 backbufferWidth, U32 backbufferHeight)
	{
		ResourceInfo.reserve(++resourceCount);
		ResourceLifetimes.reserve(resourceCount);
		AddResource(
			{
				backbufferWidth, backbufferHeight, 1, FrameResourceFlags::None,
				Settings::GetBackbufferFormat(), ColorF4(0.0f, 0.0f, 0.0f, 1.0f)
			});
	}

	RID FrameBufferDesc::AddResource(FrameResourceDesc&& info) noexcept
	{
		ZE_ASSERT(!((info.Flags & FrameResourceFlags::ForceDSV) && (info.Flags & FrameResourceFlags::SimultaneousAccess)),
			"Cannot use depth stencil with simultaneous access resource!");
		RID id = ResourceInfo.size();
		ResourceInfo.emplace_back(std::forward<FrameResourceDesc>(info));
		ResourceLifetimes.emplace_back(std::map<U64, std::pair<Resource::State, QueueType>>({}));
		return id;
	}

	void FrameBufferDesc::ComputeWorkflowTransitions(U64 dependencyLevels) noexcept
	{
		// Backbuffer states wrapping
		auto& backbuffer = ResourceLifetimes.front();
		TransitionsPerLevel.resize(dependencyLevels * 2);
		if (backbuffer.begin()->first != 0)
		{
			TransitionsPerLevel.front().emplace_back(TransitionDesc(0, BarrierType::Begin, Resource::StatePresent, backbuffer.begin()->second.first), QueueType::Main);
			TransitionsPerLevel.at(2 * backbuffer.begin()->first).emplace_back(TransitionDesc(0, BarrierType::End, Resource::StatePresent, backbuffer.begin()->second.first), QueueType::Main);
		}
		else
			TransitionsPerLevel.front().emplace_back(TransitionDesc(0, BarrierType::Immediate, Resource::StatePresent, backbuffer.begin()->second.first), QueueType::Main);

		// Cull same states between dependency levels and compute types of barriers per resource
		for (U64 i = 0; i < ResourceLifetimes.size(); ++i)
		{
			auto& res = ResourceLifetimes.at(i);
			if (res.size() > 1)
			{
				for (auto first = res.begin(), next = ++res.begin(); next != res.end();)
				{
					if (first->second.first == next->second.first)
					{
						if (next != --res.end())
						{
							auto after = next;
							if ((++after)->second.first == next->second.first)
								next = res.erase(next);
							else
							{
								first = next;
								next = after;
							}
						}
						else
							break;
					}
					else
					{
						if (next->first - first->first > 1)
						{
							if (first->second.second == next->second.second)
							{
								TransitionsPerLevel.at(2 * static_cast<U64>(first->first) + 1).emplace_back(TransitionDesc(static_cast<RID>(i), BarrierType::Begin, first->second.first, next->second.first), first->second.second);
								TransitionsPerLevel.at(2 * static_cast<U64>(next->first)).emplace_back(TransitionDesc(static_cast<RID>(i), BarrierType::End, first->second.first, next->second.first), next->second.second);
							}
							else
								TransitionsPerLevel.at(2 * static_cast<U64>(next->first)).emplace_back(TransitionDesc(static_cast<RID>(i), BarrierType::Immediate, first->second.first, next->second.first), next->second.second);
						}
						else
							TransitionsPerLevel.at(2 * static_cast<U64>(first->first) + 1).emplace_back(TransitionDesc(static_cast<RID>(i), BarrierType::Immediate, first->second.first, next->second.first), first->second.second);
						first = next++;
					}
				}
			}
		}
	}
}