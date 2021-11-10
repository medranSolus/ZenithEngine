#pragma once
#include "GFX/API/ApiType.h"

namespace ZE
{
	// Options to start Zenith Engine with
	struct EngineParams
	{
		const char* WindowName;
		GfxApiType GraphicsAPI;
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
		// When computing render graph for passes minimize distances between dependant passes.
		// Disables kicking off work earlier, trying to run pass as late as possible.
		//
		// WARNING: may result in higher or lower memory reservation for frame in some cases
		//   but can sometimes lowerage GPU consumption if most of heavy work is done early, measure it for specific use case.
		bool MinimizeRenderPassDistances;
		// Dimensions of used shadow maps
		U32 ShadowMapSize;
	};
}