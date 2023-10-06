#pragma once
#include "GFX/Pipeline/ParamsPBR.h"
#include "CmdParser.h"

namespace ZE
{
	// Options to start Zenith Engine with
	struct EngineParams
	{
		// Will be application name if not provided.
		const char* WindowName;
		// Width of client area, specify as 0 in either of dimensions to use current max resolution
		U32 Width;
		// Height of client area, specify as 0 in either of dimensions to use current max resolution
		U32 Height;
		// Number of descriptors to be created for supported graphics backend to use.
		// Determines maximal number of objects and materials
		U32 GraphicsDescriptorPoolSize;
		// Set to true if required to have performance measurements entries in single line each.
		bool SingleLinePerfEntry;
		// Parameters for used render pipeline
		GFX::Pipeline::ParamsPBR Renderer;

		static void SetupParser(CmdParser& parser) noexcept;
		static void SetParsedParams(const CmdParser& parser, EngineParams& params) noexcept;
	};
}