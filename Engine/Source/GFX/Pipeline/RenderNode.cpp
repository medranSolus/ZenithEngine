#include "GFX/Pipeline/RenderNode.h"
#include "Exception/RenderGraphCompileException.h"

namespace ZE::GFX::Pipeline
{
	void RenderNode::AddInput(std::string&& name, Resource::State state)
	{
		if (std::find(inputNames.begin(), inputNames.end(), name) != inputNames.end())
			throw ZE_RGC_EXCEPT("Pass [" + passName + "] already contains input [" + name + "]!");
		inputNames.emplace_back(std::forward<std::string>(name));
		inputStates.emplace_back(state);
	}

	void RenderNode::AddInnerBuffer(std::string&& name, Resource::State initState, FrameResourceDesc&& desc)
	{
		std::string bufferName = passName + "_" + std::forward<std::string>(name);
		if (std::find(inputNames.begin(), inputNames.end(), bufferName) != inputNames.end())
			throw ZE_RGC_EXCEPT("Pass [" + passName + "] already contains inner buffer [" + name + "]!");
		innerBuffers.emplace_back(std::move(bufferName), initState, std::forward<FrameResourceDesc>(desc));
	}

	void RenderNode::AddOutput(std::string&& name, Resource::State state, const std::string& resourceName)
	{
		std::string outputName = passName + "." + std::forward<std::string>(name);
		if (std::find(inputNames.begin(), inputNames.end(), outputName) != inputNames.end())
			throw ZE_RGC_EXCEPT("Pass [" + passName + "] already contains output [" + name + "]!");
		outputNames.emplace_back(std::move(outputName));
		outputStates.emplace_back(state);
		outputResourceNames.emplace_back(resourceName);
	}
}