#include "GFX/Pipeline/RenderNode.h"

namespace ZE::GFX::Pipeline
{
	void RenderNode::AddInput(std::string&& name, Resource::State state)
	{
		if (std::find(inputNames.begin(), inputNames.end(), name) != inputNames.end())
			throw ZE_RGC_EXCEPT("Pass [" + passName + "] already contains input [" + name + "]!");
		inputNames.emplace_back(std::forward<std::string>(name));
		inputStates.emplace_back(state);
	}

	void RenderNode::AddInnerBuffer(Resource::State initState, FrameResourceDesc&& desc) noexcept
	{
		desc.Flags |= FrameResourceFlags::ForceSRV;
		innerBuffers.emplace_back(initState, std::forward<FrameResourceDesc>(desc));
	}

	void RenderNode::AddOutput(std::string&& name, Resource::State state, RID rid)
	{
		std::string outputName = passName + "." + std::forward<std::string>(name);
		if (std::find(inputNames.begin(), inputNames.end(), outputName) != inputNames.end())
			throw ZE_RGC_EXCEPT("Pass [" + passName + "] already contains output [" + name + "]!");
		outputNames.emplace_back(std::move(outputName));
		outputStates.emplace_back(state);
		outputRIDs.emplace_back(rid);
	}

	CommandList RenderNode::GetStaticExecuteData() noexcept
	{
		ZE_ASSERT(!isStatic, "Cannot get static execute data for non-static pass!");
		ZE_ASSERT(executeData, "Execute data cannot be empty for static pass!");

		CommandList cl = std::move(*reinterpret_cast<CommandList*>(executeData));
		delete reinterpret_cast<CommandList*>(executeData);
		return cl;
	}

	RID* RenderNode::GetNodeRIDs() const noexcept
	{
		RID* rids = new RID[inputRIDs.size() + innerRIDs.size() + outputRIDs.size()];
		RID i = 0;
		for (RID rid : inputRIDs)
			rids[i++] = rid;
		for (RID rid : innerRIDs)
			rids[i++] = rid;
		for (RID rid : outputRIDs)
			rids[i++] = rid;
		return rids;
	}
}