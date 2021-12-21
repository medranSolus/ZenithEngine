#pragma once
#include "GFX/QueueType.h"
#include "GFX/Pipeline/FrameBufferDesc.h"
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
		bool isStatic;
		// Input info
		std::vector<std::string> inputNames;
		std::vector<Resource::State> inputStates;
		std::vector<U64> inputRIDs;
		// Inner buffer info
		std::vector<InnerBuffer> innerBuffers;
		std::vector<U64> innerRIDs;
		// Output info
		std::vector<std::string> outputNames;
		std::vector<Resource::State> outputStates;
		std::vector<U64> outputRIDs;

	public:
		RenderNode(std::string&& name, QueueType passType, PassExecuteCallback passExecute,
			PassCleanCallback passClean = nullptr, void* executeData = nullptr, bool isStatic = false) noexcept
			: passName(std::forward<std::string>(name)), passType(passType), passExecute(passExecute),
			passClean(passClean), executeData(executeData), isStatic(isStatic) {}
		ZE_CLASS_DEFAULT(RenderNode);
		~RenderNode() = default;

		constexpr const std::string& GetName() const noexcept { return passName; }
		constexpr const QueueType GetPassType() const noexcept { return passType; }
		constexpr PassExecuteCallback GetExecuteCallback() const noexcept { return passExecute; }
		constexpr PassCleanCallback GetCleanCallback() const noexcept { return passClean; }
		constexpr void* GetExecuteData() const noexcept { return executeData; }
		constexpr bool IsStatic() const noexcept { return isStatic; }

		constexpr const std::vector<std::string>& GetInputs() const noexcept { return inputNames; }
		constexpr std::vector<InnerBuffer>& GetInnerBuffers() noexcept { return innerBuffers; }
		constexpr const std::vector<std::string>& GetOutputs() const noexcept { return outputNames; }
		constexpr const std::vector<U64>& GetOutputResources() const noexcept { return outputRIDs; }
		constexpr Resource::State GetInputState(U64 i) const noexcept { return inputStates.at(i); }
		constexpr Resource::State GetOutputState(U64 i) const noexcept { return outputStates.at(i); }

		bool ContainsInput(const std::string& name) const noexcept { return std::find(inputNames.begin(), inputNames.end(), name) != inputNames.end(); }
		void AddInputResource(U64 rid) noexcept { inputRIDs.emplace_back(rid); }
		void AddInnerBufferResource(U64 rid) noexcept { innerRIDs.emplace_back(rid); }

		void AddInput(std::string&& name, Resource::State state);
		void AddInnerBuffer(Resource::State initState, FrameResourceDesc&& desc) noexcept;
		void AddOutput(std::string&& name, Resource::State state, U64 rid);
		CommandList GetStaticExecuteData() noexcept;
		U64* GetNodeRIDs() const noexcept;
	};
}