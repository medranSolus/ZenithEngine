#pragma once
#include "PixelFormat.h"

namespace ZE::GFX::Pipeline
{
	// Collection of modes for specified buffer
	typedef U16 FrameResourceFlags;

	// Additional info for framebuffer resource
	enum class FrameResourceFlag : U16
	{
		None = 0x0000,
		// Cannot be used together with Texture3D
		Cube = 0x0001,
		// Cannot be used together with Cube
		Texture3D = 0x0002,
		ForceSRV = 0x0004,
		// Cannot be used together with SimultaneousAccess
		ForceDSV = 0x0008,
		// Cannot be used together with ForceDSV
		SimultaneousAccess = 0x0010,
		// When creating SRV over depth stencil choose to use stencil instead of depth
		StencilView = 0x0020,
		// Resource have to preserve it's content's over the frame and cannot alias with other resources
		Temporal = 0x0040,
		// When resizing buffers sync this resource to render size (cannot be used together with SyncDisplaySize).
		// Changing `FrameResourceDesc::Sizes` into scaling values for render size.
		// If no scaling operation is selected then use render size directly
		SyncRenderSize = 0x0080,
		// When resizing buffers sync this resource to display size (cannot be used together with SyncRenderSize)
		// Changing `FrameResourceDesc::Sizes` into scaling values for display size.
		// If no scaling operation is selected then use display size directly
		SyncDisplaySize = 0x0100,
		// When syncing size of resource to external source, use `FrameResourceDesc::Sizes`
		// as multiplication factor (cannot be used together with SyncScalingDivide)
		SyncScalingMultiply = 0x0200,
		// When syncing size of resource to external source, use `FrameResourceDesc::Sizes`
		// as division factor (cannot be used together with SyncScalingMultiply)
		SyncScalingDivide = 0x0400,

		// Internal flag indicating that resource is active in current configuration
		InternalResourceActive = 0x0800,
		// Internal flag indicating that resource has render target usage in current configuration
		InternalUsageRenderTarget = 0x1000,
		// Internal flag indicating that resource has depth stencil usage in current configuration
		InternalUsageDepth = 0x2000,
		// Internal flag indicating that resource has unordered access usage in current configuration
		InternalUsageUnorderedAccess = 0x4000,
		// Internal flag indicating that resource has shader resource usage in current configuration
		InternalUsageShaderResource = 0x8000,
		// Mask for extraction of internal flags used by RenderGraphBuilder during FrameBuffer initialization
		InternalFlagsMask = InternalResourceActive | InternalUsageRenderTarget | InternalUsageDepth | InternalUsageUnorderedAccess | InternalUsageShaderResource,
	};
	ZE_ENUM_OPERATORS(FrameResourceFlag, FrameResourceFlags);

	// Description of single resource in FrameBuffer
	struct FrameResourceDesc
	{
		UInt2 Sizes;
		U16 DepthOrArraySize; // When creating as 3D texture then use it as depth
		FrameResourceFlags Flags;
		PixelFormat Format;
		ColorF4 ClearColor;
		float ClearDepth = 0.0f;
		U8 ClearStencil = 0;
		U16 MipLevels = 1;
#if _ZE_DEBUG_GFX_NAMES
		std::string DebugName = "";
#endif
	};
}

#if _ZE_DEBUG_GFX_NAMES
// Sets name to be used as indentificator of created FrameBuffer resource
#	define ZE_FRAME_RES_SET_NAME(resDesc, name) resDesc.DebugName = name
// When initializing FrameResourceDesc use this member to pass in optional debug name
#	define ZE_FRAME_RES_INIT_NAME(name) , name
#else
// Sets name to be used as indentificator of created FrameBuffer resource
#	define ZE_FRAME_RES_SET_NAME(resDesc, name)
// When initializing FrameResourceDesc use this member to pass in optional debug name
#	define ZE_FRAME_RES_INIT_NAME(name)
#endif