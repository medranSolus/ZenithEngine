#pragma once
#include "Schema.h"

namespace ZE::GFX::Binding
{
	// Data for proper binding of materials and resources in single RenderPass
	struct Context
	{
		const Schema& BindingSchema;
		U32 Count = 0;
	};
}