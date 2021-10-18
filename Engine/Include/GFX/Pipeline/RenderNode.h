#pragma once
#include "GFX/QueueType.h"
#include "GFX/Resource/FrameBufferDesc.h"
#include "PassDesc.h"

namespace ZE::GFX::Pipeline
{
	class RenderNode final
	{
	public:
		struct InnerBuffer
		{
			std::string Name;
			Resource::State InitState;
			Resource::FrameResourceDesc Info;
		};

	private:
		std::string passName;
		QueueType passType;
		PassExecuteCallback passExecute;
		void* executeData;
		// Input info
		std::vector<std::string> inputNames;
		std::vector<Resource::State> inputStates;
		// Inner buffer info
		std::vector<InnerBuffer> innerBuffers;
		// Output info
		std::vector<std::string> outputNames;
		std::vector<Resource::State> outputStates;
		std::vector<std::string> outputResourceNames;

	public:
		RenderNode(std::string&& name, QueueType passType, PassExecuteCallback passExecute, void* executeData = nullptr) noexcept
			: passName(std::forward<std::string>(name)), passType(passType), passExecute(passExecute), executeData(executeData) {}
		RenderNode(RenderNode&&) = default;
		RenderNode(const RenderNode&) = default;
		RenderNode& operator=(RenderNode&&) = default;
		RenderNode& operator=(const RenderNode&) = default;
		~RenderNode() = default;

		constexpr const std::string& GetName() const noexcept { return passName; }
		constexpr const QueueType GetPassType() const noexcept { return passType; }
		constexpr PassExecuteCallback GetExecuteCallback() const noexcept { return passExecute; }
		constexpr void* GetExecuteData() const noexcept { return executeData; }

		constexpr const std::vector<std::string>& GetInputs() const noexcept { return inputNames; }
		constexpr std::vector<InnerBuffer>& GetInnerBuffers() noexcept { return innerBuffers; }
		constexpr const std::vector<std::string>& GetOutputs() const noexcept { return outputNames; }
		constexpr const std::vector<std::string>& GetOutputResources() const noexcept { return outputResourceNames; }
		constexpr Resource::State GetOutputState(U64 i) const noexcept { return outputStates.at(i); }
		constexpr Resource::State GetInputeState(U64 i) const noexcept { return inputStates.at(i); }
		bool ContainsInput(const std::string& name) const noexcept { return std::find(inputNames.begin(), inputNames.end(), name) != inputNames.end(); }

		void AddInput(std::string&& name, Resource::State state);
		void AddInnerBuffer(std::string&& name, Resource::State initState, Resource::FrameResourceDesc&& desc);
		void AddOutput(std::string&& name, Resource::State state, const std::string& resourceName);
	};
}