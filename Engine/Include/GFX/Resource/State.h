#pragma once
#include "Types.h"

namespace ZE::GFX::Resource
{
	// Possible resource states
	enum class State : U8
	{
		// State not specified, common state
		Common,
		// Allow reads as any resource, required for upload heap
		GenericRead,
		// Frame to present on screen
		Present,

		VertexBuffer,
		ConstantBuffer,
		IndexBuffer,
		ShaderResourceNonPS,
		ShaderResourcePS,
		DepthRead,

		RenderTarget,
		UnorderedAccess,
		DepthWrite,

		CopyDestination,
		CopySource,
		ResolveDestination,
		ResolveSource,

		StreamOut,
		Indirect,
		Predication,
		AccelerationStructureRT,
		ShadingRateSource,

		VideoDecodeRead,
		VideoDecodeWrite,
		VideoProcessRead,
		VideoProcessWrite,
		VideoEncodeRead,
		VideoEncodeWrite,
	};
}