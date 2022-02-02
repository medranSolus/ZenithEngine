#pragma once
#include "Schema.h"

namespace ZE::GFX::Binding
{
	// Data for proper binding of materials and resources in single RenderPass
	struct Context
	{
		const Schema& BindingSchema;
		U32 Count = 0;

		// Set current binding slot from the end of range by some offset (0=last, 1=last-1, etc.)
		constexpr void SetFromEnd(U32 offset) noexcept { Count = BindingSchema.GetIndexFromEnd(offset); }
		constexpr void Reset() noexcept { Count = 0; }
	};
}