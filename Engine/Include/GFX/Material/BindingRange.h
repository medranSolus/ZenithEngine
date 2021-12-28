#pragma once

namespace ZE::GFX::Material
{
	// Flags changing behavior of specific range
	enum RangeFlags : U8
	{
		// Interpret range as inlined constant. Count field should contain
		// size of structure to be used with shader as Constant Buffer View
		Constant = 1,
		// Range will be used as table slot. Allows to set whole range with single call
		Material = 2,
		// Appends new range to the previous material. Must follow directly after range with same or 'Table' flag
		MaterialAppend = 4,
		// Mark range as Shader Resource View range. Cannot use with Constant flag set.
		// When not using raw or structured texture this flag have to be used only within material
		SRV = 8,
		// Mark range as Unordered Access View range. Cannot use with Constant flag set
		// When not using raw or structured buffer this flag have to be used only within material
		UAV = 16,
		// Mark range as Constant Buffer View range. Cannot use with Constant flag set
		CBV = 32,
	};

	// Single range of shader registers to use
	struct BindingRange
	{
		U32 Count;
		U32 SlotStart;
		Resource::ShaderType Shader;
		U8 Flags = 0;
	};
}
namespace ZE
{
	// Flags changing behavior of specific range
	typedef GFX::Material::RangeFlags MaterialFlags;
}