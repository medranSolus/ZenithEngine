#pragma once
#include "GFX/CommandList.h"
#include "RendererPassData.h"
#include "ResourceID.h"

namespace ZE::GFX::Pipeline
{
	// Data unique to the given render pass with all needed information
	struct PassData
	{
		Ptr<const RID> Resources;
		PtrVoid ExecData = nullptr;
	};

	// Create all needed data for render pass
	typedef void* (*PassInitCallback)(Device&, RendererPassBuildData&, const std::vector<PixelFormat>&, void*);
	// Optional function that will be performing extended setup that needs GPU explicit work
	typedef void (*PassSetupCallback)(Device&, CommandList&, RendererPassExecuteData&, PassData&, RendererPassBuildData&);
	// Evaluate whether pass shall run and if it cause update of the render graph
	typedef bool (*PassEvaluateExecutionCallback)() noexcept;
	// Main function that will be performing rendering, obligatory
	typedef void (*PassExecuteCallback)(Device&, CommandList&, RendererPassExecuteData&, PassData&);
	// Optional function to handle pass data update after render graph got it's update.
	// Can also cause render graph update when causes critical changes to the global settings (like render size for upscaling)
	typedef bool (*PassUpdatetCallback)(Device&, RendererPassBuildData&, void*, const std::vector<PixelFormat>&);
	// Optional function that will be freeing up data used by pass
	typedef void (*PassCleanCallback)(Device&, void*) noexcept;
	// Optional function for copying init data for pass creation
	typedef void* (*PassCopyInitDataCallback)(void*) noexcept;
	// Optional function for freeing up init data for pass creation
	typedef void (*PassFreeInitDataCallback)(void*) noexcept;

	// Types of every render pass present, including custom ones created outside engine
	typedef U32 PassType;
	// Enum for every type of render pass present in the engine
	enum class CorePassType : PassType
	{
		// Special type indicating that it's impossible to guess the type of the pass and it's related data
		Invalid,

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
		XeGTAO,
		SSSR,

		LightCombine,
		Skybox,

		UpscaleFSR1,
		UpscaleFSR2,
		UpscaleNIS,
		UpscaleXeSS,

		OutlineClear,
		OutlineDraw,
		HorizontalBlur,
		VerticalBlur,

		Wireframe,
		HDRGammaCorrection,
		// Begining of the range that is possible for other custom passes to use their own
		// enum values if they wish to create custom render graph definition
		CustomStart = 0x80000000
	};

	// Information about given render pass
	struct PassDesc
	{
		PassType Type = static_cast<PassType>(CorePassType::Invalid);
		// Optional data for pass intialization
		PtrVoid InitData = nullptr;
		// Optional list of pixel formats for buffers used in pass
		std::vector<PixelFormat> InitializeFormats;
		PassInitCallback Init = nullptr;
		// Optional for additional setup step with access to GPU commands
		PassSetupCallback Setup = nullptr;
		// Check whether pass should run, meaning it can be removed from execution otherwise with all further processing passes.
		// If not provided then assume always returning true
		PassEvaluateExecutionCallback Evaluate = nullptr;
		// Only required callback for pass execution
		PassExecuteCallback Execute = nullptr;
		PassUpdatetCallback Update = nullptr;
		// Required if init callback returns pointer to arbitrary data
		PassCleanCallback Clean = nullptr;
		// Required if InitData is present
		PassCopyInitDataCallback CopyInitData = nullptr;
		// Required if InitData is present
		PassFreeInitDataCallback FreeInitData = nullptr;
	};
}