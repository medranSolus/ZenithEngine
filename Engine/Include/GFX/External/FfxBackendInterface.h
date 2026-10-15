#pragma once
#include "Data/Library.h"
#include "GFX/Pipeline/FrameBuffer.h"
#include "GFX/Resource/DynamicCBuffer.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/ChainPool.h"
#include "GFX/DiskManager.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_interface.h"
ZE_WARNING_POP

namespace ZE::GFX::External::FFX
{
	// Custom extension formats to basic surfaces
	enum FfxCustomSurfaceFormat : U32
	{
		FFX_ZE_SURFACE_FORMAT_R32G32B32A32_SINT = FFX_SURFACE_FORMAT_R32_TYPELESS + 1,
		FFX_ZE_SURFACE_FORMAT_R16G16B16A16_UINT,
		FFX_ZE_SURFACE_FORMAT_R16G16B16A16_SINT,
		FFX_ZE_SURFACE_FORMAT_R16G16B16A16_UNORM,
		FFX_ZE_SURFACE_FORMAT_R16G16B16A16_SNORM,
		FFX_ZE_SURFACE_FORMAT_R8G8B8A8_UINT,
		FFX_ZE_SURFACE_FORMAT_R8G8B8A8_SINT,

		FFX_ZE_SURFACE_FORMAT_R32G32_UINT,
		FFX_ZE_SURFACE_FORMAT_R32G32_SINT,
		FFX_ZE_SURFACE_FORMAT_R16G16_UNORM,
		FFX_ZE_SURFACE_FORMAT_R16G16_SNORM,
		FFX_ZE_SURFACE_FORMAT_R8G8_SINT,
		FFX_ZE_SURFACE_FORMAT_R8G8_SNORM,

		FFX_ZE_SURFACE_FORMAT_R32_SINT,
		FFX_ZE_SURFACE_FORMAT_R16_SINT,
		FFX_ZE_SURFACE_FORMAT_R8_SINT,
		FFX_ZE_SURFACE_FORMAT_R8_SNORM,

		FFX_ZE_SURFACE_FORMAT_R10G10B10A2_UINT,
		FFX_ZE_SURFACE_FORMAT_BC6H_UF16,
		FFX_ZE_SURFACE_FORMAT_BC6H_SF16,
	};

	// Description of additional resource used by FFX backend to be allocated in frame buffer
	struct InternalResourceDescription
	{
		Pipeline::FrameResourceDesc Desc = {};
		RID ResID = INVALID_RID;
		U32 PassID = UINT32_MAX;
	};

	// Data about pass to help in creation of new resources with setting their correct lifetime
	struct PassInfo
	{
		U32 PassID = UINT32_MAX;
	};

	// ID of internally created resource
	typedef U32 ResID;

	// Main context used by FFX SDK
	struct BackendContext
	{
		entt::basic_registry<ResID> Resources;
		Data::Library<U64, Resource::PipelineStateCompute> Pipelines;
		Data::Library<U64, U64> PipelinesReferences;
		Data::Library<U64, Binding::Schema> Bindings;
		Data::Library<U64, U64> BindingsReferences;
		Data::Library<IndirectCommandType, CommandSignature> CommandSignatures;
		Data::Library<IndirectCommandType, U64> CommandSignaturesReferences;
		Data::Library<FfxEffect, FfxEffectMemoryUsage> EffectMemoryUsage;
		std::vector<Pipeline::BarrierTransition> Barriers;
		std::vector<FfxGpuJobDescription> Jobs;
	};

	// Interface data setup when filling FfxInterface
	struct BackendInterface
	{
		ChainPool<Resource::DynamicCBuffer>& DynamicBuffers;
		Pipeline::FrameBuffer& Buffers;
		DiskManager& Disk;
		Data::Library<S32, FFX::InternalResourceDescription>& InternalBuffers;
		bool& NotifyBuffersChange;
		U32 ContextRefCount = 0;
		PassInfo CurrentPass = {};
		std::unique_ptr<BackendContext> Ctx;
	};

	// Main point of interaction with FFX SDK
	struct FfxBackendInterface
	{
		FfxInterface Interface = {};
		std::unique_ptr<BackendInterface> Backend;
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
	FfxBackendInterface GetInterface(Device& dev, ChainPool<Resource::DynamicCBuffer>& dynamicBuffers, Pipeline::FrameBuffer& frameBuffer,
		DiskManager& disk, Data::Library<S32, FFX::InternalResourceDescription>& internalBuffers, bool& notifyBuffersChange) noexcept;

	// Set information about current pass for creation of internal buffers. Need to be called before and after Init/Update pass calls
	void SetCurrentPass(FfxBackendInterface& backendInterface, const PassInfo* info) noexcept;

	// Free up FFX SDK backend interface
	void DestroyInterface(FfxBackendInterface& backendInterface) noexcept;

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
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R32G32B32A32_SINT);
		case PixelFormat::R16G16B16A16_Float:
			return FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT;
		case PixelFormat::R16G16B16A16_UInt:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R16G16B16A16_UINT);
		case PixelFormat::R16G16B16A16_SInt:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R16G16B16A16_SINT);
		case PixelFormat::R16G16B16A16_UNorm:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R16G16B16A16_UNORM);
		case PixelFormat::R16G16B16A16_SNorm:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R16G16B16A16_SNORM);
		case PixelFormat::R8G8B8A8_UInt:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R8G8B8A8_UINT);
		case PixelFormat::R8G8B8A8_SInt:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R8G8B8A8_SINT);
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
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R32G32_UINT);
		case PixelFormat::R32G32_SInt:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R32G32_SINT);
		case PixelFormat::R16G16_Float:
			return FFX_SURFACE_FORMAT_R16G16_FLOAT;
		case PixelFormat::R16G16_UInt:
			return FFX_SURFACE_FORMAT_R16G16_UINT;
		case PixelFormat::R16G16_SInt:
			return FFX_SURFACE_FORMAT_R16G16_SINT;
		case PixelFormat::R16G16_UNorm:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R16G16_UNORM);
		case PixelFormat::R16G16_SNorm:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R16G16_SNORM);
		case PixelFormat::R8G8_UInt:
			return FFX_SURFACE_FORMAT_R8G8_UINT;
		case PixelFormat::R8G8_UNorm:
			return FFX_SURFACE_FORMAT_R8G8_UNORM;
		case PixelFormat::R8G8_SInt:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R8G8_SINT);
		case PixelFormat::R8G8_SNorm:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R8G8_SNORM);
		case PixelFormat::R32_Float:
		case PixelFormat::R32_Depth:
			return FFX_SURFACE_FORMAT_R32_FLOAT;
		case PixelFormat::R32_UInt:
			return FFX_SURFACE_FORMAT_R32_UINT;
		case PixelFormat::R32_SInt:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R32_SINT);
		case PixelFormat::R16_Float:
		case PixelFormat::R16_Depth:
			return FFX_SURFACE_FORMAT_R16_FLOAT;
		case PixelFormat::R16_UInt:
			return FFX_SURFACE_FORMAT_R16_UINT;
		case PixelFormat::R16_SInt:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R16_SINT);
		case PixelFormat::R16_UNorm:
			return FFX_SURFACE_FORMAT_R16_UNORM;
		case PixelFormat::R16_SNorm:
			return FFX_SURFACE_FORMAT_R16_SNORM;
		case PixelFormat::R8_UInt:
			return FFX_SURFACE_FORMAT_R8_UINT;
		case PixelFormat::R8_UNorm:
			return FFX_SURFACE_FORMAT_R8_UNORM;
		case PixelFormat::R8_SInt:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R8_SINT);
		case PixelFormat::R8_SNorm:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R8_SNORM);
		case PixelFormat::R10G10B10A2_UInt:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_R10G10B10A2_UINT);
		case PixelFormat::R10G10B10A2_UNorm:
			return FFX_SURFACE_FORMAT_R10G10B10A2_UNORM;
		case PixelFormat::R11G11B10_Float:
			return FFX_SURFACE_FORMAT_R11G11B10_FLOAT;
		case PixelFormat::R9G9B9E5_SharedExp:
			return FFX_SURFACE_FORMAT_R9G9B9E5_SHAREDEXP;
		case PixelFormat::BC6H_UF16:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_BC6H_UF16);
		case PixelFormat::BC6H_SF16:
			return static_cast<FfxSurfaceFormat>(FFX_ZE_SURFACE_FORMAT_BC6H_SF16);
		}
	}

	constexpr PixelFormat GetPixelFormat(FfxSurfaceFormat format) noexcept
	{
		switch (static_cast<U32>(format))
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
		// Extension formats
		case FFX_ZE_SURFACE_FORMAT_R32G32B32A32_SINT:
			return PixelFormat::R32G32B32A32_SInt;
		case FFX_ZE_SURFACE_FORMAT_R16G16B16A16_UINT:
			return PixelFormat::R16G16B16A16_UInt;
		case FFX_ZE_SURFACE_FORMAT_R16G16B16A16_SINT:
			return PixelFormat::R16G16B16A16_SInt;
		case FFX_ZE_SURFACE_FORMAT_R16G16B16A16_UNORM:
			return PixelFormat::R16G16B16A16_UNorm;
		case FFX_ZE_SURFACE_FORMAT_R16G16B16A16_SNORM:
			return PixelFormat::R16G16B16A16_SNorm;
		case FFX_ZE_SURFACE_FORMAT_R8G8B8A8_UINT:
			return PixelFormat::R8G8B8A8_UInt;
		case FFX_ZE_SURFACE_FORMAT_R8G8B8A8_SINT:
			return PixelFormat::R8G8B8A8_SInt;
		case FFX_ZE_SURFACE_FORMAT_R32G32_UINT:
			return PixelFormat::R32G32_UInt;
		case FFX_ZE_SURFACE_FORMAT_R32G32_SINT:
			return PixelFormat::R32G32_SInt;
		case FFX_ZE_SURFACE_FORMAT_R16G16_UNORM:
			return PixelFormat::R16G16_UNorm;
		case FFX_ZE_SURFACE_FORMAT_R16G16_SNORM:
			return PixelFormat::R16G16_SNorm;
		case FFX_ZE_SURFACE_FORMAT_R8G8_SINT:
			return PixelFormat::R8G8_SInt;
		case FFX_ZE_SURFACE_FORMAT_R8G8_SNORM:
			return PixelFormat::R8G8_SNorm;
		case FFX_ZE_SURFACE_FORMAT_R32_SINT:
			return PixelFormat::R32_SInt;
		case FFX_ZE_SURFACE_FORMAT_R16_SINT:
			return PixelFormat::R16_SInt;
		case FFX_ZE_SURFACE_FORMAT_R8_SINT:
			return PixelFormat::R8_SInt;
		case FFX_ZE_SURFACE_FORMAT_R8_SNORM:
			return PixelFormat::R8_SNorm;
		case FFX_ZE_SURFACE_FORMAT_R10G10B10A2_UINT:
			return PixelFormat::R10G10B10A2_UInt;
		case FFX_ZE_SURFACE_FORMAT_BC6H_UF16:
			return PixelFormat::BC6H_UF16;
		case FFX_ZE_SURFACE_FORMAT_BC6H_SF16:
			return PixelFormat::BC6H_SF16;
		}
	}
#pragma region
}