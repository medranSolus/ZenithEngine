#pragma once
#include "GFX/QueueType.h"
#include "PassDesc.h"

namespace ZE::GFX::Pipeline
{
	// Description of single render pass in RenderGraph
	class RenderNode final
	{
		struct InnerBuffer
		{
			Resource::State InitState;
			FrameResourceDesc Info;
		};

		std::string passName;
		QueueType passType;
		PassExecuteCallback passExecute;
		PassCleanCallback passClean;
		void* executeData;
		// Input info
		std::vector<std::pair<std::string, bool>> inputNames;
		std::vector<Resource::State> inputStates;
		std::vector<RID> inputRIDs;
		// Inner buffer info
		std::vector<InnerBuffer> innerBuffers;
		std::vector<RID> innerRIDs;
		// Output info
		std::vector<std::string> outputNames;
		std::vector<Resource::State> outputStates;
		std::vector<RID> outputRIDs;

	public:
		RenderNode(std::string&& name, QueueType passType, PassExecuteCallback passExecute,
			PassCleanCallback passClean = nullptr, void* executeData = nullptr) noexcept
			: passName(std::forward<std::string>(name)), passType(passType), passExecute(passExecute),
			passClean(passClean), executeData(executeData) {}
		ZE_CLASS_DEFAULT(RenderNode);
		~RenderNode() = default;

		constexpr const std::string& GetName() const noexcept { return passName; }
		constexpr QueueType GetPassType() const noexcept { return passType; }
		constexpr PassExecuteCallback GetExecuteCallback() const noexcept { return passExecute; }
		constexpr PassCleanCallback GetCleanCallback() const noexcept { return passClean; }
		constexpr void* GetExecuteData() const noexcept { return executeData; }

		constexpr const std::vector<std::pair<std::string, bool>>& GetInputs() const noexcept { return inputNames; }
		constexpr std::vector<InnerBuffer>& GetInnerBuffers() noexcept { return innerBuffers; }
		constexpr const std::vector<std::string>& GetOutputs() const noexcept { return outputNames; }
		constexpr const std::vector<RID>& GetOutputResources() const noexcept { return outputRIDs; }
		constexpr Resource::State GetInputState(RID i) const noexcept { return inputStates.at(i); }
		constexpr Resource::State GetOutputState(RID i) const noexcept { return outputStates.at(i); }

		bool ContainsInput(const std::string& name) const noexcept { return std::find_if(inputNames.begin(), inputNames.end(), [&name](const auto& input) { return input.first == name; }) != inputNames.end(); }
		void AddInputResource(RID rid) noexcept { inputRIDs.emplace_back(rid); }
		void AddInnerBufferResource(RID rid) noexcept { innerRIDs.emplace_back(rid); }

		void AddInput(std::string&& name, Resource::State state, bool required = true);
		void AddInnerBuffer(Resource::State initState, FrameResourceDesc&& desc) noexcept;
		void AddOutput(std::string&& name, Resource::State state, RID rid);

		// Order: input, inner, output (without already present RIDs from inputs)
		RID* GetNodeRIDs() const noexcept;
	};
}