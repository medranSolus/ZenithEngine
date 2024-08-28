#pragma once
#include "GFX/Pipeline/CoreRenderer.h"
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
		// When computing render graph for passes minimize distances between dependant passes.
		// Disables kicking off work earlier, trying to run pass as late as possible.
		//
		// WARNING: may result in higher or lower memory reservation for frame in some cases
		//   but can sometimes leverage GPU consumption if most of heavy work is done early, measure it for specific use case
		bool MinimizeRenderPassDistances = false;
		// Provide custom render graph definition if needed, otherwise fill in CoreRenderer parameters
		GFX::Pipeline::RenderGraphDesc* CustomRendererDesc = nullptr;
		// Parameters used by render pipeline
		GFX::Pipeline::CoreRenderer::Params CoreRendererParams;

		static void SetupParser(CmdParser& parser) noexcept;
		static void SetParsedParams(const CmdParser& parser, EngineParams& params) noexcept;
	};
}