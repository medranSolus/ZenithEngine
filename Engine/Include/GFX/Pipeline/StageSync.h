#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
	// All pipeline stages referenced by barrier
	typedef U32 StageSyncs;

	// Single stage describing which part of GPU pipeline is referenced
	// with meaning depending on which synchronization scope it is
	enum class StageSync : StageSyncs
	{
		// Before: no work need to complete before barrier starts (ResourceAccess::None required)
		// After: no work need to wait for before barrier completes (ResourceAccess::None required)
		None                              = 0x00000000,
		// Before: all work before barriers must finish
		// After: all work after barrier must wait
		All                               = 0x00000001,
		// Before: all graphics work before barrier must finish
		// After: all graphics work after barrier must wait
		AllGraphics                       = 0x00000002,
		// Indicates scope of processing index buffer inputs
		IndexInput                        = 0x00000004,
		// Indicates scope of processing geometry in shaders (vertex, hull, domain, geometry, mesh, amplification)
		GeometryShading                   = 0x00000008,
		// Scope of pixel shader execution
		PixelShading                      = 0x00000010,
		// Scope of compute shader execution
		ComputeShading                    = 0x00000020,
		// Scope of ray tracing pipelines execution
		RayTracing                        = 0x00000040,
		// Scope of indirect commands execution
		ExecuteIndirect                   = 0x00000080,
		// Scope of render target read/write/clear operations
		RenderTarget                      = 0x00000100,
		// Scope of depth stencil read/write/clear operations
		DepthStencil                      = 0x00000200,
		// Scope of clearing UAV targets
		ClearUnorderedAccess              = 0x00000400,
		// Scope of all copying operations
		Copy                              = 0x00000800,
		// Scope of all resolve commands
		Resolve                           = 0x00001000,
		// Scope of all predicates
		Predication                       = 0x00002000,
		// Scope of acceleration structure build commands
		RayTracingAccelerationStructBuild = 0x00004000,
		// Scope of acceleration structure copy commands
		RayTracingAccelerationStructCopy  = 0x00008000,
	};
	ZE_ENUM_OPERATORS(StageSync, StageSyncs);
}