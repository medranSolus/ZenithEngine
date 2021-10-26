#include "GFX/Pipeline/FrameBuffer.h"

namespace ZE::GFX::Pipeline
{
	std::vector<std::vector<TransitionDesc>> FrameBuffer::GetTransitionsPerLevel(FrameBufferDesc& desc) noexcept
	{
		// Cull same states between dependency levels and compute types of barriers per resource
		std::vector<std::vector<TransitionDesc>> transitionsPerLevel(desc.TransitionLevelsCount);
		for (U64 i = 0; i < desc.ResourceLifetimes.size(); ++i)
		{
			auto& res = desc.ResourceLifetimes.at(i);
			if (res.size() > 1 || i == 0)
			{
				for (auto first = res.begin(), next = ++res.begin(); next != res.end();)
				{
					if (first->second == next->second)
						next = res.erase(next);
					else
					{
						if (next->first - first->first > 1)
						{
							transitionsPerLevel.at(2 * first->first + 1).emplace_back(i, BarrierType::Begin, first->second, next->second);
							transitionsPerLevel.at(2 * next->first).emplace_back(i, BarrierType::End, first->second, next->second);
						}
						else
							transitionsPerLevel.at(2 * first->first + 1).emplace_back(i, BarrierType::Immediate, first->second, next->second);
						first = next++;
					}
				}
				if (i != 0)
				{
					// Normal resource state wrapping between frames
					Resource::State firstState = res.begin()->second;
					Resource::State lastState = res.rbegin()->second;
					if (firstState != lastState)
					{
						transitionsPerLevel.at(2 * res.begin()->first).emplace_back(i, BarrierType::End, lastState, firstState);
						transitionsPerLevel.at(2 * res.rbegin()->first + 1).emplace_back(i, BarrierType::Begin, lastState, firstState);
					}
				}
				else
				{
					// Backbuffer states wrapping
					if (res.begin()->first != 0)
					{
						transitionsPerLevel.front().emplace_back(i, BarrierType::Begin, Resource::State::Present, res.begin()->second);
						transitionsPerLevel.at(2 * res.begin()->first).emplace_back(i, BarrierType::End, Resource::State::Present, res.begin()->second);
					}
					else
						transitionsPerLevel.front().emplace_back(i, BarrierType::Immediate, Resource::State::Present, res.begin()->second);
					transitionsPerLevel.back().emplace_back(i, BarrierType::Immediate, res.rbegin()->second, Resource::State::Present);
				}
			}
		}
		return transitionsPerLevel;
	}
}