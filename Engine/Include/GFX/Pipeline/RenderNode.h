#pragma once
#include "PassDesc.h"
#include "TextureLayout.h"

namespace ZE::GFX::Pipeline
{
	// How pass will execute based on input data, controls how it's evaluation will influence graph update
	enum class PassExecutionType : U8
	{
		Producer,        // Always present if evaluation indicates so, otherwise removed (causing hard reset)
		DynamicProducer, // Present in the graph if there is enough data to process
		Processor,       // Only present if there is input data from other passes and if any further passes consume it's output data
	};

	// Description of single render pass in RenderGraph
	class RenderNode final
	{
		std::string graphName;
		std::string passName;
		PassDesc desc;
		bool async;
		PassExecutionType execType;
		// Just a hint if possible
		std::string scheduleAfter = "";
		// Input info
		std::vector<std::string> inputNames;
		std::vector<bool> inputRequired;
		std::vector<TextureLayout> inputLayouts;
		std::vector<RID> inputRIDs;
		// Inner buffer info
		std::vector<FrameResourceDesc> innerBuffers;
		std::vector<TextureLayout> innerLayouts;
		std::vector<RID> innerRIDs;
		// Output info
		std::vector<std::string> outputNames;
		std::vector<TextureLayout> outputLayouts;
		std::vector<RID> outputRIDs;
		std::vector<RID> replacementOutputRIDs;

	public:
		constexpr RenderNode(std::string&& graphName, std::string&& passName, PassDesc&& desc, PassExecutionType execType, bool async = false) noexcept
			: graphName(std::forward<std::string>(graphName)), passName(std::forward<std::string>(passName)), desc(std::forward<PassDesc>(desc)), async(async), execType(execType) {}
		ZE_CLASS_DEFAULT(RenderNode);
		~RenderNode() = default;

		constexpr const std::string& GetGraphConnectorName() const noexcept { return graphName; }
		constexpr const std::string& GetPassName() const noexcept { return passName; }
		constexpr std::string GetFullName() const noexcept { return graphName + (passName.size() ? "." + passName : ""); }
		constexpr PassDesc& GetDesc() noexcept { return desc; }
		constexpr bool IsAsync() const noexcept { return async; }
		constexpr PassExecutionType GetExecType() const noexcept { return execType; }
		constexpr void ScheduleAfter(const std::string& pass) noexcept { scheduleAfter = pass; }
		constexpr const std::string& GetPreceedingPass() const noexcept { return scheduleAfter; }

		constexpr const std::vector<std::string>& GetInputs() const noexcept { return inputNames; }
		constexpr const std::vector<FrameResourceDesc>& GetInnerBuffers() const noexcept { return innerBuffers; }
		constexpr const std::vector<std::string>& GetOutputs() const noexcept { return outputNames; }

		constexpr bool IsInputRequired(ResIndex index) const noexcept { return inputRequired.at(index); }
		constexpr TextureLayout GetInputLayout(ResIndex index) const noexcept { return inputLayouts.at(index); }
		constexpr TextureLayout GetInnerBufferLayout(ResIndex index) const noexcept { return innerLayouts.at(index); }
		constexpr TextureLayout GetOutputLayout(ResIndex index) const noexcept { return outputLayouts.at(index); }
		constexpr RID GetOutput(ResIndex index) const noexcept { return outputRIDs.at(index); }
		constexpr RID GetOutputReplacement(ResIndex index) const noexcept { return replacementOutputRIDs.at(index); }

		constexpr void SetInputResource(ResIndex index, RID rid) noexcept { inputRIDs.at(index) = rid; }
		constexpr void SetInnerBuffer(ResIndex index, RID rid) noexcept { innerRIDs.at(index) = rid; }
		bool ContainsInput(std::string_view name) const noexcept { return std::find(inputNames.begin(), inputNames.end(), name) != inputNames.end(); }

		bool AddInput(std::string&& name, TextureLayout layout, bool required = true) noexcept;
		void AddInnerBuffer(TextureLayout layout, FrameResourceDesc&& resDesc) noexcept;
		// In case of not running the pass you can provide a replacement buffer that will take over the output in graph data flow computation
		// By default it will be searched via same RID but it's possible to specify other buffer instead
		// so all further passes that reference same buffer in continuous flow will refer to the replacement buffer instead
		bool AddOutput(std::string&& name, TextureLayout layout, RID rid, RID replacement = INVALID_RID) noexcept;

		// Order: input, inner, output (without already present RIDs from inputs)
		std::unique_ptr<RID[]> GetNodeRIDs() const noexcept;
	};
}