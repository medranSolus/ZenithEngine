#include "GFX/Pipeline/FrameBufferDesc.h"
#include "Exception/RenderGraphCompileException.h"
#include "Settings.h"

namespace ZE::GFX::Pipeline
{
	void FrameBufferDesc::Init(U64 resourceCount, const char* backbufferName, U32 backbufferWidth, U32 backbufferHeight)
	{
		++resourceCount;
		ResourceNames.reserve(resourceCount);
		ResourceInfo.reserve(resourceCount);
		ResourceLifetimes.reserve(resourceCount);
		AddResource(backbufferName, { backbufferWidth, backbufferHeight, 1, { { Settings::GetBackbufferFormat(), ColorF4(0.0f, 0.0f, 0.0f, 1.0f) } } });
	}

	void FrameBufferDesc::AddResource(std::string&& name, FrameResourceDesc&& info)
	{
		if (std::find(ResourceNames.begin(), ResourceNames.end(), name) != ResourceNames.end())
			throw ZE_RGC_EXCEPT("Frame Buffer already contains resource [" + name + "]!");
		ResourceNames.emplace_back(std::forward<std::string>(name));
		ResourceInfo.emplace_back(std::forward<FrameResourceDesc>(info));
		ResourceLifetimes.emplace_back(std::map<U64, Resource::State>({}));
	}

	void FrameBufferDesc::ComputeTransitions(U64 dependencyLevels) noexcept
	{
		TransitionsPerLevel.resize(dependencyLevels * 2);
		// Cull same states between dependency levels and compute types of barriers per resource
		for (U64 i = 0; i < ResourceLifetimes.size(); ++i)
		{
			auto& res = ResourceLifetimes.at(i);
			if (res.size() > 1)
			{
				for (auto first = res.begin(), next = ++res.begin(); next != res.end();)
				{
					if (first->second == next->second)
						next = res.erase(next);
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
		// Normal resource state wrapping between frames
		for (U64 i = 1; i < ResourceLifetimes.size(); ++i)
		{
			auto& res = ResourceLifetimes.at(i);
			if (res.size() > 1)
			{
				Resource::State firstState = res.begin()->second;
				Resource::State lastState = res.rbegin()->second;
				if (firstState != lastState)
				{
					TransitionsPerLevel.at(2 * res.begin()->first).emplace_back(i, BarrierType::End, lastState, firstState);
					TransitionsPerLevel.at(2 * res.rbegin()->first + 1).emplace_back(i, BarrierType::Begin, lastState, firstState);
				}
			}
		}
		// Backbuffer states wrapping
		auto& backbuffer = ResourceLifetimes.front();
		if (backbuffer.begin()->first != 0)
		{
			TransitionsPerLevel.front().emplace_back(0, BarrierType::Begin, Resource::State::Present, backbuffer.begin()->second);
			TransitionsPerLevel.at(2 * backbuffer.begin()->first).emplace_back(0, BarrierType::End, Resource::State::Present, backbuffer.begin()->second);
		}
		else
			TransitionsPerLevel.front().emplace_back(0, BarrierType::Immediate, Resource::State::Present, backbuffer.begin()->second);
		TransitionsPerLevel.back().emplace_back(0, BarrierType::Immediate, backbuffer.rbegin()->second, Resource::State::Present);
	}
}