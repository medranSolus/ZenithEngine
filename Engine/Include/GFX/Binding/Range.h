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
		BufferPackAppend = 4,
		// When data is not changing during execution of the pipeline it can be marked as static for internal driver optimizations.
		// Should only be used with ranges not belonging to resources created by pipeline! Automatically set when flags 'Constant' or 'CBV' are set.
		// Warning: Schema does not perform checks on this flag as it's impossible to determine if current range is using FrameBuffer data!
		StaticData = 8,
		// When CBuffer is defined as global in shader code use this flag for proper shader space generation for supported APIs.
		GlobalBuffer = 16,
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
		Resource::ShaderTypes Shaders;
		RangeFlags Flags = 0;

		constexpr void Validate() const noexcept;
	};

#pragma region Functions
	constexpr void Range::Validate() const noexcept
	{
		ZE_ASSERT(((Flags & Constant) == 0 || (Flags & BufferPack) == 0)
			&& ((Flags & Constant) == 0 || (Flags & BufferPackAppend) == 0)
			&& ((Flags & BufferPack) == 0 || (Flags & BufferPackAppend) == 0),
			"Single range should only have one of the flags: Constant, BufferPack or BufferPackAppends!");
		ZE_ASSERT((Flags & Constant) == 0
			|| (Flags & (SRV | UAV | CBV)) == 0,
			"Flags SRV, UAV or CBV cannot be specified with flag Constant!");
		ZE_ASSERT(((Flags & SRV) == 0 || (Flags & UAV) == 0)
			&& ((Flags & SRV) == 0 || (Flags & CBV) == 0)
			&& ((Flags & UAV) == 0 || (Flags & CBV) == 0),
			"Single range should only have one of the flags: SRV, UAV or CBV!");
		ZE_ASSERT(Count != 0, "There should be at least 1 resource!");
	}
#pragma endregion
}