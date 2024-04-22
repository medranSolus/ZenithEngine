#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
	// For what type of accesses texture memory is formatted
	enum class TextureLayout : U8
	{
		Undefined, // Can only be used as LayoutBefore since it's indicating that any data in resource can be discarded
		Preinitialized, // Can only be used as LayoutBefore since it's indicating that data in resource has been written by host and have to be preserved
		Common,
		Present,
		GenericRead,
		RenderTarget,
		UnorderedAccess,
		DepthStencilWrite,
		DepthStencilRead,
		ShaderResource,
		CopySource,
		CopyDest,
		ResolveSource,
		ResolveDest,
		ShadingRateSource
	};
}