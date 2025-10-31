#pragma once
#include "PassDesc.h"
#include "TextureLayout.h"

namespace ZE::GFX::Pipeline
{
	// How pass will execute based on input data, controls how it's evaluation will influence graph update
	enum class PassExecutionType : U8
	{
		Producer,         // Always present if evaluation indicates so, otherwise removed
		Processor,        // Only present if there is input data from other passes and if any further passes consume it's output data
		StaticProcessor,  // Same as normal processor pass but different evaluation is causing a framegraph rebuilding (same as Producer)
		DynamicProcessor, // Same as normal processor pass but different evaluation is not causing a framebuffer recomputation (it's buffers are present always if there are producers), have to be alone in pass group
		Startup,          // Run only once at the start of the render graph if it's output resources are present in framebuffer, can contain only output buffers
	};

	// Description of single render pass in RenderGraph
	class RenderNode final
	{
		std::string graphName;
		std::string passName;
		PassDesc desc;
		// Async | GFX pass | compute pass | RT pass | No exec data caching | Init GPU upload required
		std::bitset<6> flags;
		PassExecutionType execType;
		// Just a hint if possible
		std::string scheduleAfter = "";
		// Input info
		std::vector<std::string> inputNames;
		std::vector<bool> inputRequired;
		std::vector<TextureLayout> inputLayouts;
		// Inner buffer info
		std::vector<FrameResourceDesc> innerBuffers;
		std::vector<TextureLayout> innerLayouts;
		// Output info
		std::vector<std::string> outputNames;
		std::vector<TextureLayout> outputLayouts;
		std::vector<std::string> outputResources;
		std::vector<std::string> replacementOutputResources;

	public:
		constexpr RenderNode(std::string&& graphName, std::string&& passName, PassDesc&& desc, PassExecutionType execType, bool async = false) noexcept
			: graphName(std::forward<std::string>(graphName)), passName(std::forward<std::string>(passName)), desc(std::forward<PassDesc>(desc)), flags(async), execType(execType) {}
		ZE_CLASS_MOVE_ONLY(RenderNode);
		RenderNode(const RenderNode& node) noexcept { *this = node; }
		RenderNode& operator=(const RenderNode& node) noexcept;
		~RenderNode();

		constexpr const std::string& GetGraphConnectorName() const noexcept { return graphName; }
		constexpr const std::string& GetPassName() const noexcept { return passName; }
		constexpr std::string GetFullName() const noexcept { return graphName + (passName.size() ? "." + passName : ""); }
		constexpr const PassDesc& GetDesc() const noexcept { return desc; }
		constexpr bool IsAsync() const noexcept { return flags[0]; }
		// When pass execution data cannot be used by multiple instances of same pass then prevent it with this flag (no effect for startup passes)
		constexpr void DisableExecDataCaching() noexcept { flags[4] = true; }
		constexpr bool IsExecDataCachingDisabled() const noexcept { return flags[4]; }
		// When pass during initialization needs to upload some data to the GPU (ex. textures) set this flag to allow for correct synchronization
		constexpr void SetInitDataGpuUploadRequired() noexcept { flags[5] = true; }
		constexpr bool IsInitDataGpuUploadRequired() const noexcept { return flags[5]; }
		constexpr PassExecutionType GetExecType() const noexcept { return execType; }
		constexpr void SetProducer() noexcept { execType = PassExecutionType::Producer; }
		constexpr void ScheduleAfter(const std::string& pass) noexcept { scheduleAfter = pass; }
		constexpr const std::string& GetPreceedingPass() const noexcept { return scheduleAfter; }

		// Set of hints allowing better barrier placement accoriding to type of work performed
		constexpr void SetHintGfx() noexcept { flags[1] = true; }
		constexpr bool IsGfxPass() const noexcept { return flags[1]; }
		constexpr void SetHintCompute() noexcept { flags[2] = true; }
		constexpr bool IsComputePass() const noexcept { return flags[2]; }
		// Only is using classic RT pipelines
		constexpr void SetHintRayTracing() noexcept { flags[3] = true; }
		constexpr bool IsRayTracingPass() const noexcept { return flags[3]; }

		constexpr const std::vector<std::string>& GetInputs() const noexcept { return inputNames; }
		constexpr const std::vector<bool>& GetInputRequirements() const noexcept { return inputRequired; }
		constexpr const std::vector<FrameResourceDesc>& GetInnerBuffers() const noexcept { return innerBuffers; }
		constexpr const std::vector<std::string>& GetOutputs() const noexcept { return outputNames; }
		constexpr const std::vector<std::string>& GetOutputResources() const noexcept { return outputResources; }
		constexpr const std::vector<std::string>& GetOutputReplacementResources() const noexcept { return replacementOutputResources; }

		constexpr bool IsInputRequired(ResIndex index) const noexcept { return inputRequired.at(index); }
		constexpr TextureLayout GetInputLayout(ResIndex index) const noexcept { return inputLayouts.at(index); }
		constexpr TextureLayout GetInnerBufferLayout(ResIndex index) const noexcept { return innerLayouts.at(index); }
		constexpr TextureLayout GetOutputLayout(ResIndex index) const noexcept { return outputLayouts.at(index); }
		constexpr std::string GetInnerBufferName(ResIndex index) const noexcept { return GetFullName() + "." + std::to_string(index); }

		bool ContainsInput(std::string_view name) const noexcept { return std::find(inputNames.begin(), inputNames.end(), name) != inputNames.end(); }

		bool AddInput(std::string&& name, TextureLayout layout, bool required = true) noexcept;
		void AddInnerBuffer(TextureLayout layout, FrameResourceDesc&& resDesc) noexcept;
		// In case of not running the pass you can provide a replacement buffer that will take over the output in graph data flow computation
		// By default it will be searched via same name but it's possible to specify other buffer instead
		// so all further passes that reference same buffer in continuous flow will refer to the replacement buffer instead
		bool AddOutput(std::string&& name, TextureLayout layout, std::string_view resourceName, std::string_view replacement = "") noexcept;
	};
}