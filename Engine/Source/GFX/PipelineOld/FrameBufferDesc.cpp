#include "Settings.h"

namespace ZE::GFX::Pipeline
{
	void ComputeWorkflowTransitions(U64 dependencyLevels) noexcept
	{
		/*
		// Backbuffer states wrapping
		auto& backbuffer = ResourceLifetimes.front();
		TransitionsPerLevel.resize(dependencyLevels * 2);
		if (backbuffer.begin()->first != 0)
		{
			TransitionsPerLevel.front().emplace_back(static_cast<RID>(0), BarrierType::Begin, Resource::StatePresent, backbuffer.begin()->second);
			TransitionsPerLevel.at(2 * backbuffer.begin()->first).emplace_back(static_cast<RID>(0), BarrierType::End, Resource::StatePresent, backbuffer.begin()->second);
		}
		else
			TransitionsPerLevel.front().emplace_back(static_cast<RID>(0), BarrierType::Immediate, Resource::StatePresent, backbuffer.begin()->second);

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
						{
							auto after = next;
							if ((++after)->second == next->second)
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
							TransitionsPerLevel.at(2 * Utils::SafeCast<U64>(first->first) + 1).emplace_back(Utils::SafeCast<RID>(i), BarrierType::Begin, first->second, next->second);
							TransitionsPerLevel.at(2 * Utils::SafeCast<U64>(next->first)).emplace_back(Utils::SafeCast<RID>(i), BarrierType::End, first->second, next->second);
						}
						else
							TransitionsPerLevel.at(2 * Utils::SafeCast<U64>(first->first) + 1).emplace_back(Utils::SafeCast<RID>(i), BarrierType::Immediate, first->second, next->second);
						first = next++;
					}
				}
			}
		}
		*/
	}
}