#pragma once

namespace ZE::GFX
{
	// Type of Tonemapper used in the rendering pipeline
	enum class TonemapperType : U8
	{
		Reinhard,
		Exposure,
	};
}