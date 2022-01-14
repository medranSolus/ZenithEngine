#pragma once

namespace ZE::GFX::Binding
{
	// Data for proper binding of materials and resources in single RenderPass
	struct Context
	{
		// Maybe add also Schema to access during binding 
		U32 Count = 0;
	};
}