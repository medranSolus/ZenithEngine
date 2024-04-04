#pragma once
#include "PixelFormat.h"

namespace ZE::GFX::Pipeline
{
	// Collection of modes for specified buffer
	typedef U16 FrameResourceFlags;

	// Additional info for framebuffer resource
	enum class FrameResourceFlag : U16
	{
		None                = 0x0000,
		Cube                = 0x0001,
		ForceSRV            = 0x0002,
		// Cannot be used together with SimultaneousAccess
		ForceDSV            = 0x0004,
		// Cannot be used together with ForceDSV
		SimultaneousAccess  = 0x0008,
		// When creating SRV over depth stencil choose to use stencil instead of depth
		StencilView         = 0x0010,
		// Resource have to preserve it's content's over the frame
		Temporal            = 0x0020,
		// When resizing buffers sync this resource to render size (cannot be used together with SyncDisplaySize).
		// Changing `FrameResourceDesc::Sizes` into scaling values for render size.
		// If no scaling operation is selected then use render size directly
		SyncRenderSize      = 0x0040,
		// When resizing buffers sync this resource to display size (cannot be used together with SyncRenderSize)
		// Changing `FrameResourceDesc::Sizes` into scaling values for display size.
		// If no scaling operation is selected then use display size directly
		SyncDisplaySize     = 0x0080,
		// When syncing size of resource to external source, use `FrameResourceDesc::Sizes`
		// as multiplication factor (cannot be used together with SyncScalingDivide)
		SyncScalingMultiply = 0x0100,
		// When syncing size of resource to external source, use `FrameResourceDesc::Sizes`
		// as division factor (cannot be used together with SyncScalingMultiply)
		SyncScalingDivide   = 0x0200,
	};
	ZE_ENUM_OPERATORS(FrameResourceFlag, FrameResourceFlags);

	// Description of single resource in FrameBuffer
	struct FrameResourceDesc
	{
		UInt2 Sizes;
		U16 ArraySize;
		FrameResourceFlags Flags;
		PixelFormat Format;
		ColorF4 ClearColor;
		float ClearDepth = 0.0f;
		U8 ClearStencil = 0;
		U16 MipLevels = 1;
	};
}