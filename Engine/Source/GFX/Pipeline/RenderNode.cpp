#include "GFX/Pipeline/RenderNode.h"

namespace ZE::GFX::Pipeline
{
	RenderNode::RenderNode(std::string&& name, QueueType passType, PassExecuteCallback passExecute,
		PassCleanCallback passClean, void* executeData) noexcept
		: passName(std::forward<std::string>(name)), passType(passType), passExecute(passExecute),
		passClean(passClean), executeData(executeData)
	{
		ZE_ASSERT(passType != QueueType::Copy,
			"Copy queue not supported in render passes! Consider using Compute queue for async copying of data and Copy queue for outside of renderer copies.");
	}
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

	RID* RenderNode::GetNodeRIDs() const noexcept
	{
		std::vector<RID> out = outputRIDs;
		for (RID rid : inputRIDs)
		{
			auto it = std::find(out.begin(), out.end(), rid);
			if (it != out.end())
				out.erase(it);
		}

		RID* rids = new RID[inputRIDs.size() + innerRIDs.size() + out.size()];
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