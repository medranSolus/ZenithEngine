#pragma once
#include "GFX/Resource/ShaderType.h"

namespace ZE::GFX::Binding
{
	typedef U8 RangeFlags;
	// Flags changing behavior of specific range
	enum RangeFlag : RangeFlags
	{
		// Interpret range as inlined constant. Count field should contain
		// size of structure to be used with shader as Constant Buffer View
		Constant = 1,
		// Range will be used as a table slot. Allows to set whole range with single call
		BufferPack = 2,
		// Appends new range to the previous buffer pack. Must follow directly after range with same or 'BufferPack' flag
		BufferPackAppend = 8,
		// When data is not changing during execution of the pipeline it can be marked as static for internal driver optimizations.
		// Should only be used with ranges not belonging to resources created by pipeline! Automatically set when flags 'Constant' or 'CBV' are set.
		// Warning: Schema does not perform checks on this flag as it's impossible to determine if current range is using FrameBuffer data!
		StaticData = 16,
		// Mark range as Shader Resource View range. Cannot be used with Constant flag set.
		// When not using raw or structured texture this flag have to be used only inside buffer pack
		SRV = 32,
		// Mark range as Unordered Access View range. Cannot be used with Constant flag set.
		// When not using raw or structured buffer this flag have to be used only inside buffer pack
		UAV = 64,
		// Mark range as Constant Buffer View range. Cannot use with Constant flag set
		CBV = 128,
	};

	// Single range of shader registers to use
	struct Range
	{
		U32 Count;
		U32 StartSlot;
		Resource::ShaderType Shader;
		RangeFlags Flags = 0;
	};
}