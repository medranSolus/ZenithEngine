#pragma once

namespace ZE::GFX
{
	// Indication which Ray Tracing tier is supported
	enum class RayTracingTier : U8
	{
		None, V1_0, V1_1
	};
}