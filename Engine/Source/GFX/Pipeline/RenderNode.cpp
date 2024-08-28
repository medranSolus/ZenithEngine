#include "GFX/Pipeline/RenderNode.h"

namespace ZE::GFX::Pipeline
{
	RenderNode& RenderNode::operator=(const RenderNode& node) noexcept
	{
		graphName = node.graphName;
		passName = node.passName;
		desc = node.desc;
		// Copy since RenderNode manages lifetime of InitData
		if (desc.InitData)
		{
			ZE_ASSERT(desc.CopyInitData, "CopyInitData callback not provided!");
			desc.InitData = desc.CopyInitData ? desc.CopyInitData(node.desc.InitData) : nullptr;
		}

		flags = node.flags;
		execType = node.execType;
		scheduleAfter = node.scheduleAfter;
		inputNames = node.inputNames;
		inputRequired = node.inputRequired;
		inputLayouts = node.inputLayouts;
		innerBuffers = node.innerBuffers;
		innerLayouts = node.innerLayouts;
		outputNames = node.outputNames;
		outputLayouts = node.outputLayouts;
		outputResources = node.outputResources;
		replacementOutputResources = node.replacementOutputResources;
		return *this;
	}

	RenderNode::~RenderNode()
	{
		if (desc.InitData)
		{
			if (desc.FreeInitData)
				desc.FreeInitData(desc.InitData);
			else
			{
				ZE_FAIL("Cannot clear initialization data for pass [" + GetFullName() + "], no FreeInitData callback provided! Memory leak detected!");
			}
		}
	}

	bool RenderNode::AddInput(std::string&& name, TextureLayout layout, bool required) noexcept
	{
#if _ZE_RENDERER_CREATION_VALIDATION
		if (ContainsInput(name))
		{
			ZE_FAIL("Pass [" + GetFullName() + "] already contains input [" + name + "]!");
			return false;
		}
#endif
		inputNames.emplace_back(std::forward<std::string>(name));
		inputRequired.emplace_back(required);
		inputLayouts.emplace_back(layout);
		return true;
	}

	void RenderNode::AddInnerBuffer(TextureLayout layout, FrameResourceDesc&& resDesc) noexcept
	{
		resDesc.Flags |= FrameResourceFlag::ForceSRV;
		innerBuffers.emplace_back(std::forward<FrameResourceDesc>(resDesc));
		innerLayouts.emplace_back(layout);
	}

	bool RenderNode::AddOutput(std::string&& name, TextureLayout layout, std::string_view resourceName, std::string_view replacement) noexcept
	{
		std::string outputName = graphName + "." + std::forward<std::string>(name);
#if _ZE_RENDERER_CREATION_VALIDATION
		if (std::find(outputNames.begin(), outputNames.end(), outputName) != outputNames.end())
		{
			ZE_FAIL("Pass [" + GetFullName() + "] already contains output [" + name + "]!");
			return false;
		}
#endif
		outputNames.emplace_back(std::move(outputName));
		outputLayouts.emplace_back(layout);
		outputResources.emplace_back(resourceName);
		replacementOutputResources.emplace_back(replacement);
		return true;
	}
}