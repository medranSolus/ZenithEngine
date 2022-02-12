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
		RID id = ResourceInfo.size();
		ResourceInfo.emplace_back(std::forward<FrameResourceDesc>(info));
		ResourceLifetimes.emplace_back(std::map<RID, Resource::State>({}));
		return id;
	}

	void FrameBufferDesc::ComputeWorkflowTransitions(U64 dependencyLevels) noexcept
	{
		// Backbuffer states wrapping
		auto& backbuffer = ResourceLifetimes.front();
		TransitionsPerLevel.resize(dependencyLevels * 2);
		if (backbuffer.begin()->first != 0)
		{
			TransitionsPerLevel.front().emplace_back(0, BarrierType::Begin, Resource::State::Present, backbuffer.begin()->second);
			TransitionsPerLevel.at(2 * backbuffer.begin()->first).emplace_back(0, BarrierType::End, Resource::State::Present, backbuffer.begin()->second);
		}
		else
			TransitionsPerLevel.front().emplace_back(0, BarrierType::Immediate, Resource::State::Present, backbuffer.begin()->second);

		// Cull same states between dependency levels and compute types of barriers per resource
		for (U64 i = 0; i < ResourceLifetimes.size(); ++i)
		{
			auto& res = ResourceLifetimes.at(i);
			if (res.size() > 1)
			{
				for (auto first = res.begin(), next = ++res.begin(); next != res.end();)
				{
					if (first->second == next->second)
					{
						if (next != --res.end())
							next = res.erase(next);
						else
							break;
					}
					else
					{
						if (next->first - first->first > 1)
						{
							TransitionsPerLevel.at(2 * first->first + 1).emplace_back(i, BarrierType::Begin, first->second, next->second);
							TransitionsPerLevel.at(2 * next->first).emplace_back(i, BarrierType::End, first->second, next->second);
						}
						else
							TransitionsPerLevel.at(2 * first->first + 1).emplace_back(i, BarrierType::Immediate, first->second, next->second);
						first = next++;
					}
				}
			}
		}
	}
}