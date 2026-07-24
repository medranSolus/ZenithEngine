#pragma once
#include "GFX/CommandList.h"
#include "RendererPassData.h"
#include "ResourceID.h"

namespace ZE::GFX::Pipeline
{
	// Data unique to the given render pass with all needed information
	struct PassData
	{
		std::unique_ptr<RID[]> Resources;
		std::shared_ptr<PassExecuteData> ExecData;
	};
	
	// Base class for optional data used during pass initialization
	struct PassInitData
	{
		PassInitData() = default;
		ZE_CLASS_DEFAULT(PassInitData);
		virtual ~PassInitData() = default;

		virtual std::unique_ptr<PassInitData> Clone() const noexcept = 0;
	};

	// Information about how update was performed for pass
	enum class UpdateOperation : U8
	{
		NoUpdate,                    // Nothing updated
		InternalOnly,                // Only internals of the pass were updated
		GpuUploadRequired,           // If update resulted in additional data sent to GPU then communicate to graph that waiting for upload is required
		GraphImpact,                 // Updates inside pass might affect other passes as well, required to update all passes additionally
		FrameBufferImpact,           // Same as 'GraphImpact' but also frame buffer need to be recreated
		FrameBufferImpactGpuUpload,  // 'FrameBufferImpact' and 'GpuUploadRequired' combined
	};

	// Shorthand for return type expected to be returned by init callback
	typedef Expected<std::shared_ptr<PassExecuteData>> ExpectedPassExecuteData;
	// Create all needed data for render pass
	typedef ExpectedPassExecuteData (*PassInitCallback)(Device&, RendererPassBuildData&, const std::vector<PixelFormat>&, PassInitData*) noexcept;
	// Optional function to call after pass initialization when FrameBuffer resources have been created to finish preparing pass data with correct resources
	// Meant for passes that need to place additional data in the FrameBuffer before starting work
	// Will be called only once after all startup passes have finished and should return true if any commands have been recorded
	typedef Expected<bool> (*PassPrepareCallback)(Device&, CommandList&, RendererPassExecuteData&, PassData&) noexcept;
	// Evaluate whether pass shall run and if it cause update of the render graph
	typedef bool (*PassEvaluateExecutionCallback)() noexcept;
	// Main function that will be performing rendering, obligatory, returns true if any commands have been recorded
	typedef Expected<bool> (*PassExecuteCallback)(Device&, CommandList&, RendererPassExecuteData&, PassData&) noexcept;
	// Optional function to handle pass data update after render graph got it's update.
	// Can also cause render graph update when causes critical changes to the global settings (like render size for upscaling)
	typedef Expected<UpdateOperation> (*PassUpdateCallback)(Device&, RendererPassBuildData&, PassExecuteData*, const std::vector<PixelFormat>&) noexcept;
	// Optional function for creating ImGui debug controls
	typedef void (*PassDebugUICallback)(PassExecuteData*) noexcept;

	// Types of every render pass present, including custom ones created outside engine
	typedef U32 PassType;
	// Enum for every type of render pass present in the engine
	enum class CorePassType : PassType
	{
		// Special type indicating that it's impossible to guess the type of the pass and it's related data
		Invalid,

		LoadLightmapsDiffuse,
		LoadLightmapsSpecular,
		LoadSkybox,

		GBufferClear,
		MotionVectorsClear,
		ReactiveMaskClear,
		Lambertian,
		LambertianComputeCopy,

		LightClear,
		DirectionalLight,
		SpotLight,
		PointLight,

		CACAO,
		HBAO,
		XeGTAO,
		SSSR,

		LightCombine,
		Skybox,

		UpscaleDLSS,
		UpscaleFSR1,
		UpscaleFSR2,
		UpscaleFSR3,
		UpscaleFfxFSR,
		UpscaleNIS,
		UpscaleXeSS,

		OutlineClear,
		OutlineDraw,
		HorizontalBlur,
		VerticalBlur,

		TonemapAgX,
		TonemapCollection,
		TonemapGT7,
		TonemapLPM,
		TonemapReinhard,
		TonemapReinhardX,
		TonemapVDR,
		TonemapLPMSceneCopy,

		Wireframe,
		DearImGui,
		DebugView,
		// Begining of the range that is possible for other custom passes to use their own
		// enum values if they wish to create custom render graph definition
		CustomStart = 0x80000000
	};
	ZE_ENUM_OPERATORS(CorePassType, PassType);

	// Information about given render pass
	struct PassDesc final
	{
		PassType Type = Base(CorePassType::Invalid);
		// Optional data for pass intialization
		std::unique_ptr<PassInitData> InitData;
		// Optional list of pixel formats for buffers used in pass
		std::vector<PixelFormat> InitializeFormats;
		PassInitCallback Init = nullptr;
		PassPrepareCallback Prepare = nullptr;
		// Check whether pass should run, meaning it can be removed from execution otherwise with all further processing passes.
		// If not provided then assume always returning true
		PassEvaluateExecutionCallback Evaluate = nullptr;
		// Only required callback for pass execution
		PassExecuteCallback Execute = nullptr;
		PassUpdateCallback Update = nullptr;
		// Only used in non-release builds or in demo/editor
		PassDebugUICallback DebugUI = nullptr;

		PassDesc() = default;
		constexpr PassDesc(PassType type) noexcept : Type(type) {}
		ZE_CLASS_MOVE_ONLY(PassDesc);
		PassDesc(const PassDesc& desc) noexcept { *this = desc; }
		PassDesc& operator=(const PassDesc& desc) noexcept;
		~PassDesc() = default;
	};
}