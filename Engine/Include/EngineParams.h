#pragma once
#include "GFX/API/ApiType.h"
#include "GFX/Pipeline/ParamsPBR.h"

namespace ZE
{
	// Options to start Zenith Engine with
	struct EngineParams
	{
		const char* WindowName;
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
	};
}