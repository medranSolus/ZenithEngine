#include "GFX/Pipeline/RenderNode.h"

namespace ZE::GFX::Pipeline
{
	bool RenderNode::AddInput(std::string&& name, TextureLayout layout, bool required) noexcept
	{
#if _ZE_RENDERER_CREATION_VALIDATION
		if (ContainsInput(name))
		{
			ZE_FAIL("Pass [" + passName + "] already contains input [" + name + "]!");
			return false;
		}
#endif
		inputNames.emplace_back(std::forward<std::string>(name));
		inputRequired.emplace_back(required);
		inputLayouts.emplace_back(layout);
		inputRIDs.emplace_back(INVALID_RID);
		return true;
	}

	void RenderNode::AddInnerBuffer(TextureLayout layout, FrameResourceDesc&& resDesc) noexcept
	{
		resDesc.Flags |= FrameResourceFlag::ForceSRV;
		innerBuffers.emplace_back(std::forward<FrameResourceDesc>(resDesc));
		innerLayouts.emplace_back(layout);
		innerRIDs.emplace_back(INVALID_RID);
	}

	bool RenderNode::AddOutput(std::string&& name, TextureLayout layout, RID rid, RID replacement) noexcept
	{
		std::string outputName = passName + "." + std::forward<std::string>(name);
#if _ZE_RENDERER_CREATION_VALIDATION
		if (std::find(outputNames.begin(), outputNames.end(), outputName) != outputNames.end())
		{
			ZE_FAIL("Pass [" + passName + "] already contains output [" + name + "]!");
			return false;
		}
#endif
		outputNames.emplace_back(std::move(outputName));
		outputLayouts.emplace_back(layout);
		outputRIDs.emplace_back(rid);
		replacementOutputRIDs.emplace_back(replacement);
		return true;
	}

	std::unique_ptr<RID[]> RenderNode::GetNodeRIDs() const noexcept
	{
		std::vector<RID> out = outputRIDs;
		for (RID rid : inputRIDs)
		{
			auto it = std::find(out.begin(), out.end(), rid);
			if (it != out.end())
				out.erase(it);
		}

		auto rids = std::make_unique<RID[]>(inputRIDs.size() + innerRIDs.size() + out.size());
		RID i = 0;
		for (RID rid : inputRIDs)
			rids[i++] = rid;
		for (RID rid : innerRIDs)
			rids[i++] = rid;
		for (RID rid : out)
			rids[i++] = rid;
		return rids;
	}
}