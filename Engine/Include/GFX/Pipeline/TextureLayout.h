#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
	// For what type of accesses texture memory is formatted
	enum class TextureLayout : U8
	{
		Undefined,
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