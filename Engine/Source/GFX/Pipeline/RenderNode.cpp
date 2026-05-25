#include "GFX/Pipeline/RenderNode.h"

namespace ZE::GFX::Pipeline
{
	void RenderNode::AddInput(std::string&& name, TextureLayout layout, bool required) noexcept
	{
		ZE_ASSERT(execType != PassExecutionType::Startup, "Startup pass cannot have any inputs!");
#if _ZE_RENDERER_CREATION_VALIDATION
		if (ContainsInput(name))
		{
			ZE_FAIL("Pass [" + GetFullName() + "] already contains input [" + name + "]!");
			return;
		}
#endif
		inputNames.emplace_back(std::forward<std::string>(name));
		inputRequired.emplace_back(required);
		inputLayouts.emplace_back(layout);
	}

	void RenderNode::AddInnerBuffer(TextureLayout layout, FrameResourceDesc&& resDesc) noexcept
	{
		ZE_ASSERT(execType != PassExecutionType::Startup, "Startup pass cannot have any inner buffers!");
		resDesc.Flags |= FrameResourceFlag::ForceSRV;
		innerBuffers.emplace_back(std::forward<FrameResourceDesc>(resDesc));
		innerLayouts.emplace_back(layout);
	}

	void RenderNode::AddOutput(std::string&& name, TextureLayout layout, std::string_view resourceName, std::string_view replacement) noexcept
	{
		std::string outputName = graphName + "." + std::forward<std::string>(name);
#if _ZE_RENDERER_CREATION_VALIDATION
		if (std::find(outputNames.begin(), outputNames.end(), outputName) != outputNames.end())
		{
			ZE_FAIL("Pass [" + GetFullName() + "] already contains output [" + outputName + "]!");
			return;
		}
		if (replacement != "" && execType == PassExecutionType::Startup)
		{
			ZE_FAIL("Startup pass [" + GetFullName() + "] cannot have replacement specified for output resource!");
			return;
		}
#endif
		outputNames.emplace_back(std::move(outputName));
		outputLayouts.emplace_back(layout);
		outputResources.emplace_back(resourceName);
		replacementOutputResources.emplace_back(replacement);
	}
}