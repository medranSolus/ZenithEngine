#pragma once
#include "Data/Library.h"
#include "IO/DiskManager.h"
#include "Pipeline/FrameBuffer.h"
#include "Resource/DynamicCBuffer.h"
#include "ChainPool.h"
#include "FfxException.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_types.h"
ZE_WARNING_POP

namespace ZE::GFX::FFX
{
	// Description of additional resource used by FFX backend to be allocated in frame buffer
	struct InternalResourceDescription
	{
		Pipeline::FrameResourceDesc Desc;
		RID ResID;
		U32 PassID;
	};

	// Data about pass to help in creation of new resources with setting their correct lifetime
	struct PassInfo
	{
		U32 PassID = UINT32_MAX;
	};

	// Convert command list into handle used by FFX SDK
	constexpr FfxCommandList GetCommandList(CommandList& cl) noexcept { return (FfxCommandList)&cl; }
	// Convert pixel format into FFX SDK surface format
	constexpr FfxSurfaceFormat GetSurfaceFormat(PixelFormat format) noexcept;
	// Convert FFX SDK surface format into pixel format
	constexpr PixelFormat GetPixelFormat(FfxSurfaceFormat format) noexcept;

	// Initialize handle for FFX SDK from one of the frame buffers
	FfxResource GetResource(Pipeline::FrameBuffer& buffers, RID rid, FfxResourceStates state) noexcept;

	// Fill up pointers to FFX SDK backend callbacks
	FfxInterface GetInterface(Device& dev, ChainPool<Resource::DynamicCBuffer>& dynamicBuffers, Pipeline::FrameBuffer& frameBuffer,
		IO::DiskManager& disk, Data::Library<S32, FFX::InternalResourceDescription>& internalBuffers, bool& notifyBuffersChange) noexcept;

	// Set information about current pass for creation of internal buffers. Need to be called before and after Init/Update pass calls
	void SetCurrentPass(FfxInterface& backendInterface, const PassInfo* info) noexcept;

	// Free up FFX SDK backend interface
	void DestroyInterface(FfxInterface& backendInterface) noexcept;

#pragma region Functions
	constexpr FfxSurfaceFormat GetSurfaceFormat(PixelFormat format) noexcept
	{
		switch (format)
		{
		default:
			ZE_FAIL("Format not yet supported by FidelityFX SDK!");
			[[fallthrough]];
		case PixelFormat::Unknown:
			return FFX_SURFACE_FORMAT_UNKNOWN;
		case PixelFormat::R32G32B32A32_Float:
			return FFX_SURFACE_FORMAT_R32G32B32A32_FLOAT;
		case PixelFormat::R32G32B32A32_UInt:
			return FFX_SURFACE_FORMAT_R32G32B32A32_UINT;
		case PixelFormat::R32G32B32A32_SInt:
			ZE_WARNING("FidelityFX SDK is not supporting plain R32G32B32A32_SInt so falling back to typeless version!");
			return FFX_SURFACE_FORMAT_R32G32B32A32_TYPELESS;
		case PixelFormat::R16G16B16A16_Float:
			return FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT;
		case PixelFormat::R16G16B16A16_UInt:
		case PixelFormat::R16G16B16A16_SInt:
		case PixelFormat::R16G16B16A16_UNorm:
		case PixelFormat::R16G16B16A16_SNorm:
			ZE_WARNING("FidelityFX SDK is not supporting plain R16G16B16A16_* so falling back to typeless version!");
			return FFX_SURFACE_FORMAT_R16G16B16A16_TYPELESS;
		case PixelFormat::R8G8B8A8_UInt:
		case PixelFormat::R8G8B8A8_SInt:
			ZE_WARNING("FidelityFX SDK is not supporting plain R32G32B32A32_SInt so falling back to typeless version!");
			return FFX_SURFACE_FORMAT_R8G8B8A8_TYPELESS;
		case PixelFormat::R8G8B8A8_UNorm:
			return FFX_SURFACE_FORMAT_R8G8B8A8_UNORM;
		case PixelFormat::R8G8B8A8_UNorm_SRGB:
			return FFX_SURFACE_FORMAT_R8G8B8A8_SRGB;
		case PixelFormat::R8G8B8A8_SNorm:
			return FFX_SURFACE_FORMAT_R8G8B8A8_SNORM;
		case PixelFormat::B8G8R8A8_UNorm:
			return FFX_SURFACE_FORMAT_B8G8R8A8_UNORM;
		case PixelFormat::B8G8R8A8_UNorm_SRGB:
			return FFX_SURFACE_FORMAT_B8G8R8A8_SRGB;
		case PixelFormat::R32G32B32_Float:
			return FFX_SURFACE_FORMAT_R32G32B32_FLOAT;
		case PixelFormat::R32G32_Float:
			return FFX_SURFACE_FORMAT_R32G32_FLOAT;
		case PixelFormat::R32G32_UInt:
		case PixelFormat::R32G32_SInt:
			ZE_WARNING("FidelityFX SDK is not supporting plain R32G32_*Int so falling back to typeless version!");
			return FFX_SURFACE_FORMAT_R32G32_TYPELESS;
		case PixelFormat::R16G16_Float:
			return FFX_SURFACE_FORMAT_R16G16_FLOAT;
		case PixelFormat::R16G16_UInt:
			return FFX_SURFACE_FORMAT_R16G16_UINT;
		case PixelFormat::R16G16_SInt:
			return FFX_SURFACE_FORMAT_R16G16_SINT;
		case PixelFormat::R16G16_UNorm:
		case PixelFormat::R16G16_SNorm:
			ZE_WARNING("FidelityFX SDK is not supporting plain R16G16_*Norm so falling back to typeless version!");
			return FFX_SURFACE_FORMAT_R16G16_TYPELESS;
		case PixelFormat::R8G8_UInt:
			return FFX_SURFACE_FORMAT_R8G8_UINT;
		case PixelFormat::R8G8_UNorm:
			return FFX_SURFACE_FORMAT_R8G8_UNORM;
		case PixelFormat::R8G8_SInt:
		case PixelFormat::R8G8_SNorm:
			ZE_WARNING("FidelityFX SDK is not supporting plain R8G8_S* so falling back to typeless version!");
			return FFX_SURFACE_FORMAT_R8G8_TYPELESS;
		case PixelFormat::R32_Float:
		case PixelFormat::R32_Depth:
			return FFX_SURFACE_FORMAT_R32_FLOAT;
		case PixelFormat::R32_UInt:
			return FFX_SURFACE_FORMAT_R32_UINT;
		case PixelFormat::R32_SInt:
			ZE_WARNING("FidelityFX SDK is not supporting R32_SInt so falling back to typeless version!");
			return FFX_SURFACE_FORMAT_R32_TYPELESS;
		case PixelFormat::R16_Float:
		case PixelFormat::R16_Depth:
			return FFX_SURFACE_FORMAT_R16_FLOAT;
		case PixelFormat::R16_UInt:
			return FFX_SURFACE_FORMAT_R16_UINT;
		case PixelFormat::R16_SInt:
			ZE_WARNING("FidelityFX SDK is not supporting plain R16_SInt so falling back to typeless version!");
			return FFX_SURFACE_FORMAT_R16_TYPELESS;
		case PixelFormat::R16_UNorm:
			return FFX_SURFACE_FORMAT_R16_UNORM;
		case PixelFormat::R16_SNorm:
			return FFX_SURFACE_FORMAT_R16_SNORM;
		case PixelFormat::R8_UInt:
			return FFX_SURFACE_FORMAT_R8_UINT;
		case PixelFormat::R8_UNorm:
			return FFX_SURFACE_FORMAT_R8_UNORM;
		case PixelFormat::R8_SInt:
		case PixelFormat::R8_SNorm:
			ZE_WARNING("FidelityFX SDK is not supporting plain R8_S* so falling back to typeless version!");
			return FFX_SURFACE_FORMAT_R8_TYPELESS;
		case PixelFormat::R10G10B10A2_UInt:
			ZE_WARNING("FidelityFX SDK is not supporting plain R10G10B10A2_UInt so falling back to typeless version!");
			return FFX_SURFACE_FORMAT_R10G10B10A2_TYPELESS;
		case PixelFormat::R10G10B10A2_UNorm:
			return FFX_SURFACE_FORMAT_R10G10B10A2_UNORM;
		case PixelFormat::R11G11B10_Float:
			return FFX_SURFACE_FORMAT_R11G11B10_FLOAT;
		case PixelFormat::R9G9B9E5_SharedExp:
			return FFX_SURFACE_FORMAT_R9G9B9E5_SHAREDEXP;
		}
	}

	constexpr PixelFormat GetPixelFormat(FfxSurfaceFormat format) noexcept
	{
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FFX_SURFACE_FORMAT_UNKNOWN:
			return PixelFormat::Unknown;
		case FFX_SURFACE_FORMAT_R32G32B32A32_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R32G32B32A32_UInt!");
			[[fallthrough]];
		case FFX_SURFACE_FORMAT_R32G32B32A32_UINT:
			return PixelFormat::R32G32B32A32_UInt;
		case FFX_SURFACE_FORMAT_R32G32B32A32_FLOAT:
			return PixelFormat::R32G32B32A32_Float;
		case FFX_SURFACE_FORMAT_R16G16B16A16_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R16G16B16A16_Float!");
			[[fallthrough]];
		case FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT:
			return PixelFormat::R16G16B16A16_Float;
		case FFX_SURFACE_FORMAT_R32G32B32_FLOAT:
			return PixelFormat::R32G32B32_Float;
		case FFX_SURFACE_FORMAT_R32G32_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R32G32_Float!");
			[[fallthrough]];
		case FFX_SURFACE_FORMAT_R32G32_FLOAT:
			return PixelFormat::R32G32_Float;
		case FFX_SURFACE_FORMAT_R8_UINT:
			return PixelFormat::R8_UInt;
		case FFX_SURFACE_FORMAT_R32_UINT:
			return PixelFormat::R32_UInt;
		case FFX_SURFACE_FORMAT_R8G8B8A8_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R8G8B8A8_UInt!");
			return PixelFormat::R8G8B8A8_UInt;
		case FFX_SURFACE_FORMAT_R8G8B8A8_UNORM:
			return PixelFormat::R8G8B8A8_UNorm;
		case FFX_SURFACE_FORMAT_R8G8B8A8_SNORM:
			return PixelFormat::R8G8B8A8_SNorm;
		case FFX_SURFACE_FORMAT_R8G8B8A8_SRGB:
			return PixelFormat::R8G8B8A8_UNorm_SRGB;
		case FFX_SURFACE_FORMAT_B8G8R8A8_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding B8G8R8A8_UNorm!");
			[[fallthrough]];
		case FFX_SURFACE_FORMAT_B8G8R8A8_UNORM:
			return PixelFormat::B8G8R8A8_UNorm;
		case FFX_SURFACE_FORMAT_B8G8R8A8_SRGB:
			return PixelFormat::B8G8R8A8_UNorm_SRGB;
		case FFX_SURFACE_FORMAT_R11G11B10_FLOAT:
			return PixelFormat::R11G11B10_Float;
		case FFX_SURFACE_FORMAT_R10G10B10A2_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R10G10B10A2_UNorm!");
			[[fallthrough]];
		case FFX_SURFACE_FORMAT_R10G10B10A2_UNORM:
			return PixelFormat::R10G10B10A2_UNorm;
		case FFX_SURFACE_FORMAT_R16G16_FLOAT:
			return PixelFormat::R16G16_Float;
		case FFX_SURFACE_FORMAT_R16G16_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R16G16_UInt!");
			[[fallthrough]];
		case FFX_SURFACE_FORMAT_R16G16_UINT:
			return PixelFormat::R16G16_UInt;
		case FFX_SURFACE_FORMAT_R16G16_SINT:
			return PixelFormat::R16G16_SInt;
		case FFX_SURFACE_FORMAT_R16_FLOAT:
			return PixelFormat::R16_Float;
		case FFX_SURFACE_FORMAT_R16_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R16_UInt!");
			[[fallthrough]];
		case FFX_SURFACE_FORMAT_R16_UINT:
			return PixelFormat::R16_UInt;
		case FFX_SURFACE_FORMAT_R16_UNORM:
			return PixelFormat::R16_UNorm;
		case FFX_SURFACE_FORMAT_R16_SNORM:
			return PixelFormat::R16_SNorm;
		case FFX_SURFACE_FORMAT_R8_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R8_UNorm!");
			[[fallthrough]];
		case FFX_SURFACE_FORMAT_R8_UNORM:
			return PixelFormat::R8_UNorm;
		case FFX_SURFACE_FORMAT_R8G8_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R8G8_UNorm!");
			[[fallthrough]];
		case FFX_SURFACE_FORMAT_R8G8_UNORM:
			return PixelFormat::R8G8_UNorm;
		case FFX_SURFACE_FORMAT_R8G8_UINT:
			return PixelFormat::R8G8_UInt;
		case FFX_SURFACE_FORMAT_R32_TYPELESS:
			ZE_WARNING("Typeless format detected, falling back to corresponding R32_FloatR32_Float!");
			[[fallthrough]];
		case FFX_SURFACE_FORMAT_R32_FLOAT:
			return PixelFormat::R32_Float;
		case FFX_SURFACE_FORMAT_R9G9B9E5_SHAREDEXP:
			return PixelFormat::R9G9B9E5_SharedExp;
		}
	}
#pragma region
}