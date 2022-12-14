#pragma once
#include "GFX/API/ApiType.h"
#include "GFX/Pipeline/ParamsPBR.h"
#include "CmdParser.h"

namespace ZE
{
	// Options to start Zenith Engine with
	struct EngineParams
	{
		// Name of the application to register for external services
		const char* AppName;
		// Will be AppName if not provided
		const char* WindowName;
		// Identificator of application current version. For convenience you can use ZE::Utils::MakeVersion()
		uint32_t AppVersion;
		GfxApiType GraphicsAPI;
		// Number of backbuffers to create for swap chain, must be in range [2:16]
		U32 BackbufferCount;
		// Width of client area, specify as 0 in either of dimensions to use current max resolution
		U32 Width;
		// Height of client area, specify as 0 in either of dimensions to use current max resolution
		U32 Height;
		// Number of descriptors to be created for supported graphics backend to use.
		// Determines maximal number of objects and materials
		U32 GraphicsDescriptorPoolSize;
		// Number of descriptors in pool to use for objects rendering,
		// higher number means less possible materials but more objects can be rendered.
		// Have to be smaller than GraphicsDescriptorPoolSize!
		U32 ScratchDescriptorCount;
		// Parameters for used render pipeline
		GFX::Pipeline::ParamsPBR Renderer;

		static void SetupParser(CmdParser& parser) noexcept;
		static GfxApiType GetParsedApi(const CmdParser& parser, GfxApiType defApi = GfxApiType::DX12) noexcept;
	};
}