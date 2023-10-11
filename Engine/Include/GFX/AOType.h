#pragma once

namespace ZE::GFX
{
	// Type of Ambient Occlusion to be used in the pipeline
	enum class AOType : U8
	{
		None,
		XeGTAO,
		CACAO
	};
}