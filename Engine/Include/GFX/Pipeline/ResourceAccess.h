#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
	// All possible resource accesses to be included in barrier
	typedef U32 ResourceAccesses;

	// Type of resource access that can be made by the GPU
	enum class ResourceAccess : ResourceAccesses
	{
		None                              = 0x00000000,
		Common                            = 0x00000001,
		VertexBuffer                      = 0x00000002,
		ConstantBuffer                    = 0x00000004,
		IndexBuffer                       = 0x00000008,
		RenderTarget                      = 0x00000010,
		UnorderedAccess                   = 0x00000020,
		DepthStencilWrite                 = 0x00000040,
		DepthStencilRead                  = 0x00000080,
		ShaderResource                    = 0x00000100,
		StreamOutput                      = 0x00000200,
		IndirectArguments                 = 0x00000400,
		Predication                       = 0x00000800,
		CopyDest                          = 0x00001000,
		CopySource                        = 0x00002000,
		ResolveDest                       = 0x00004000,
		ResolveSource                     = 0x00008000,
		RayTracingAccelerationStructRead  = 0x00010000,
		RayTracingAccelerationStructWrite = 0x00020000,
		ShadingRateSource                 = 0x00040000
	};
	ZE_ENUM_OPERATORS(ResourceAccess, ResourceAccesses);
}