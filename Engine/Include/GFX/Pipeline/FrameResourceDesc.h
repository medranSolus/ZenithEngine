#pragma once
#include "Settings.h"

namespace ZE::GFX::Pipeline
{
	// Collection of modes for specified buffer
	typedef U32 FrameResourceFlags;

	// Additional info for framebuffer resource
	enum class FrameResourceFlag : FrameResourceFlags
	{
		None = 0x0000,
		// When resizing buffers sync this resource to render size (cannot be used together with SyncDisplaySize).
		// Changing `FrameResourceDesc::Sizes` into scaling values for render size.
		// If no scaling operation is selected then use render size directly
		SyncRenderSize = 0x0001,
		// When resizing buffers sync this resource to display size (cannot be used together with SyncRenderSize)
		// Changing `FrameResourceDesc::Sizes` into scaling values for display size.
		// If no scaling operation is selected then use display size directly
		SyncDisplaySize = 0x0002,
		// When syncing size of resource to external source, use `FrameResourceDesc::Sizes`
		// as multiplication factor (cannot be used together with SyncScalingDivide)
		SyncScalingMultiply = 0x0004,
		// When syncing size of resource to external source, use `FrameResourceDesc::Sizes`
		// as division factor (cannot be used together with SyncScalingMultiply)
		SyncScalingDivide = 0x0008,
		// Cannot be used together with ForceDSV
		ForceRTV = 0x0010,
		// Cannot be used together with SimultaneousAccess
		ForceDSV = 0x0020,
		// Cannot be used together with ForceDSV
		ForceUAV = 0x0040,
		ForceSRV = 0x0080,
		// Resource have to preserve it's content's over the frame and cannot alias with other resources
		Temporal = 0x0100,
		// Cannot be used together with ForceDSV
		SimultaneousAccess = 0x0200,
		// When creating SRV over depth stencil choose to use stencil instead of depth
		StencilView = 0x0400,
		// When creating SRV or UAV over texture resource, treat it as an array
		ArrayView = 0x0800,
		// When creating SRV over buffer resource, treat it as a raw view (byte address buffer)
		RawBufferView = 0x1000,
		// When creating buffer, allow it to hold indirect rendering arguments
		AllowIndirect = 0x2000,
		// Treat resource as memory region to be reserved inside FrameBuffer without actual resource allocation.
		// Sizes::X should hold lower (LSB) part of U64 size of memory region in bytes and Sizes::Y should hold upper (MSB) part
		NoResourceCreation = 0x4000,
		// Similar to NoResourceCreation but disables reserving memory for this resource indicating that it's held outside the FrameBuffer.
		// Such resource needs to be registered into FrameBuffer before it can be first used. Only supports SRV resources
		OutsideResource = 0x8000,

		// Internal flag indicating that resource is active in current configuration
		InternalResourceActive = 0x08000000,
		// Internal flag indicating that resource has render target usage in current configuration
		InternalUsageRenderTarget = 0x10000000,
		// Internal flag indicating that resource has depth stencil usage in current configuration
		InternalUsageDepth = 0x20000000,
		// Internal flag indicating that resource has unordered access usage in current configuration
		InternalUsageUnorderedAccess = 0x40000000,
		// Internal flag indicating that resource has shader resource usage in current configuration
		InternalUsageShaderResource = 0x80000000,
		// Mask for extraction of internal flags used by RenderGraphBuilder during FrameBuffer initialization
		InternalFlagsMask = InternalResourceActive | InternalUsageRenderTarget | InternalUsageDepth | InternalUsageUnorderedAccess | InternalUsageShaderResource,
	};
	ZE_ENUM_OPERATORS(FrameResourceFlag, FrameResourceFlags);

	// Type of the resource to be created
	enum class FrameResourceType : U8
	{
		// When creating Buffer resource use FrameResourceDesc::Sizes::Y as requested byte stride in views (0 typed view, otherwise structured view)
		Buffer,
		Texture1D, Texture2D, TextureCube, Texture3D
	};

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
		FrameResourceType Type = FrameResourceType::Texture2D;
#if _ZE_DEBUG_GFX_NAMES
		std::string DebugName = "";
#endif

		constexpr UInt2 GetResolutionAdjustedSizes() const noexcept;
	};

#pragma region Functions
	constexpr UInt2 FrameResourceDesc::GetResolutionAdjustedSizes() const noexcept
	{
		if (Flags & (FrameResourceFlag::SyncDisplaySize | FrameResourceFlag::SyncRenderSize))
		{
			ZE_ASSERT(((Flags & FrameResourceFlag::SyncDisplaySize) != 0) != ((Flags & FrameResourceFlag::SyncRenderSize) != 0),
				"Cannot synchronize frame resource size to display and render size at the same time!");
			ZE_ASSERT(!(((Flags & FrameResourceFlag::SyncScalingMultiply) != 0) && ((Flags & FrameResourceFlag::SyncScalingDivide) != 0)),
				"Cannot use frame resource synchronization factor as multiplier and divider at the same time!");

			UInt2 baseSize = Flags & FrameResourceFlag::SyncDisplaySize ? Settings::DisplaySize : Settings::RenderSize;
			if (Flags & FrameResourceFlag::SyncScalingMultiply)
			{
				ZE_ASSERT_WARN(Sizes.X != 0 || Sizes.Y != 0, "Warning, resolution multiplication scaling factor contains zero!");
				return { baseSize.X * Sizes.X, baseSize.Y * Sizes.Y };
			}
			if (Flags & FrameResourceFlag::SyncScalingDivide)
			{
				ZE_ASSERT(Sizes.X != 0 || Sizes.Y != 0, "Resolution division scaling factor contains zero!");
				return { baseSize.X / Sizes.X, baseSize.Y / Sizes.Y };
			}
			return baseSize;
		}
		return Sizes;
	}
#pragma endregion
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