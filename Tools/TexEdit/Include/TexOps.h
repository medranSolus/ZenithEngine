#pragma once
#include "GFX/Surface.h"

using namespace ZE;

namespace TexOps
{
	// Simple per-pixel processing of a surface
	void SimpleProcess(GFX::Surface& surface, U32 cores, bool noAlpha, bool flipY) noexcept;

	// Converts an equirectangular HDRi surface to a cubemap surface
	void ConvertToCubemap(const GFX::Surface& surface, GFX::Surface& cubemap, U32 cores, bool bilinear, bool fp16) noexcept;
}