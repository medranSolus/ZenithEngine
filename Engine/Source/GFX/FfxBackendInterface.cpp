#include "GFX/FfxBackendInterface.h"
#include "GFX/Resource/Generic.h"
#include "Data/Library.h"

// Assert for FFX backend interface pointer
#define ZE_CHECK_FFX_BACKEND() ZE_ASSERT(backendInterface, "Empty FFX backend interface!")

namespace ZE::GFX
{
	struct FfxBackendInterface
	{
		S32 NextResourceIndex = 0;
		Data::Library<S32, Resource::Generic> InternalResources;
	};

	// Interface functions used by FFX SDK backend
	FfxUInt32 GetSDKVersion(FfxInterface* backendInterface);
	FfxErrorCode CreateBackendContext(FfxInterface* backendInterface, FfxUInt32* effectContextId);
	FfxErrorCode GetDeviceCapabilities(FfxInterface* backendInterface, FfxDeviceCapabilities* deviceCapabilities);
	FfxErrorCode DestroyBackendContext(FfxInterface* backendInterface, FfxUInt32 effectContextId);
	FfxErrorCode CreateResource(FfxInterface* backendInterface,
		const FfxCreateResourceDescription* createResourceDescription,
		FfxUInt32 effectContextId, FfxResourceInternal* outTexture);
	FfxErrorCode ExecuteGpuJobs(FfxInterface* backendInterface, FfxCommandList commandList);

	// Utility functions for working with FFX SDK
	constexpr FfxBackendInterface& GetFfxCtx(FfxInterface* backendInterface) noexcept;
	constexpr Device& GetDevice(FfxInterface* backendInterface) noexcept;
	constexpr Resource::GenericResourceType GetResourceType(FfxResourceType type) noexcept;
	constexpr Resource::GenericResourceHeap GetHeapType(FfxHeapType type) noexcept;
	constexpr Resource::GenericResourceFlags GetResourceFlags(FfxResourceUsage usage) noexcept;
	constexpr PixelFormat GetPixelFormat(FfxSurfaceFormat format) noexcept;
	constexpr Resource::State GetState(FfxResourceStates state) noexcept;

	void ffxGetInterface(FfxInterface& backendInterface, Device& dev) noexcept
	{
		backendInterface.fpGetSDKVersion = GetSDKVersion;
		backendInterface.fpCreateBackendContext = CreateBackendContext;
		backendInterface.fpGetDeviceCapabilities = GetDeviceCapabilities;
		backendInterface.fpDestroyBackendContext = DestroyBackendContext;
		backendInterface.fpCreateResource = CreateResource;
		//backendInterface->fpDestroyResource = DestroyResourceDX12;
		//backendInterface->fpGetResource = GetResourceDX12;
		//backendInterface->fpRegisterResource = RegisterResourceDX12;
		//backendInterface->fpUnregisterResources = UnregisterResourcesDX12;
		//backendInterface->fpGetResourceDescription = GetResourceDescriptorDX12;
		//backendInterface->fpCreatePipeline = CreatePipelineDX12;
		//backendInterface->fpDestroyPipeline = DestroyPipelineDX12;
		//backendInterface->fpScheduleGpuJob = ScheduleGpuJobDX12;
		//backendInterface->fpExecuteGpuJobs = ExecuteGpuJobsDX12;

		// Memory assignments
		backendInterface.scratchBufferSize = sizeof(FfxBackendInterface);
		backendInterface.scratchBuffer = new U8[backendInterface.scratchBufferSize];

		// Set the device
		backendInterface.device = ffxGetDevice(dev);
	}

#pragma region FFX backend functions
	FfxUInt32 GetSDKVersion(FfxInterface* backendInterface)
	{
		return FFX_SDK_MAKE_VERSION(FFX_SDK_VERSION_MAJOR, FFX_SDK_VERSION_MINOR, FFX_SDK_VERSION_PATCH);
	}

	FfxErrorCode CreateBackendContext(FfxInterface* backendInterface, FfxUInt32* effectContextId)
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(backendInterface->scratchBuffer, "Empty FFX backend context memory!");

		// Not using effect IDs anyway
		if (effectContextId)
			*effectContextId = 0;

		FfxBackendInterface* ctx = reinterpret_cast<FfxBackendInterface*>(backendInterface->scratchBuffer);
		std::memset(ctx, 0, sizeof(FfxBackendInterface));
		new(ctx) FfxBackendInterface;

		return FFX_OK;
	}

	FfxErrorCode GetDeviceCapabilities(FfxInterface* backendInterface, FfxDeviceCapabilities* deviceCapabilities)
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(deviceCapabilities, "Empty FFX device capabilities!");

		Device& dev = GetDevice(backendInterface);
		switch (dev.GetMaxShaderModel())
		{
		case ShaderModel::V5_0:
			ZE_WARNING("No option to specify lower shader model in FFX SDK than 5.1 so in case of older APIs assume 5.1");
			[[fallthrough]];
		case ShaderModel::V5_1:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_5_1;
			break;
		case ShaderModel::V6_0:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_0;
			break;
		case ShaderModel::V6_1:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_1;
			break;
		case ShaderModel::V6_2:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_2;
			break;
		case ShaderModel::V6_3:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_3;
			break;
		case ShaderModel::V6_4:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_5;
			break;
		case ShaderModel::V6_5:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_5;
			break;
		case ShaderModel::V6_6:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_6;
			break;
		case ShaderModel::V6_7:
		case ShaderModel::V6_8:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_7;
			break;
		}

		auto minMax = dev.GetWaveLaneCountRange();
		deviceCapabilities->waveLaneCountMin = minMax.first;
		deviceCapabilities->waveLaneCountMax = minMax.second;
		deviceCapabilities->fp16Supported = dev.IsShaderFloat16Supported();
		deviceCapabilities->raytracingSupported = Settings::RayTracingTier != GFX::RayTracingTier::None;

		return FFX_OK;
	}

	FfxErrorCode DestroyBackendContext(FfxInterface* backendInterface, FfxUInt32 effectContextId)
	{
		ZE_ASSERT(backendInterface, "Empty FFX backend interface!");

		FfxBackendInterface& ctx = GetFfxCtx(backendInterface);

		// Free all remaining resources
		Device& dev = GetDevice(backendInterface);
		ctx.InternalResources.Transform([&dev](Resource::Generic& res) { res.Free(dev); });

		ctx.~FfxBackendInterface();
		return FFX_OK;
	}

	FfxErrorCode CreateResource(FfxInterface* backendInterface,
		const FfxCreateResourceDescription* createResourceDescription,
		FfxUInt32 effectContextId, FfxResourceInternal* outTexture)
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(createResourceDescription, "Empty FFX resource description");
		ZE_ASSERT(outTexture, "Empty FFX out texture!");

		// FFX_RESOURCE_FLAGS_ALIASABLE -> make use of it somewhere here or rework Framebuffer to make use of this resource
		// FFX_RESOURCE_FLAGS_UNDEFINED -> only for Vulkan meaning that there is no source data and first barrier must provide layout as undefined (maybe for new DX12 barriers too)

		Device& dev = GetDevice(backendInterface);
		FfxBackendInterface& ctx = GetFfxCtx(backendInterface);

		outTexture->internalIndex = ++ctx.NextResourceIndex;
		if (ctx.NextResourceIndex == 0)
		{
			ZE_FAIL("FFX resource index wrap around!");
			return FFX_ERROR_OUT_OF_MEMORY;
		}

		Resource::GenericResourceDesc resDesc = {};
		resDesc.Type = GetResourceType(createResourceDescription->resourceDescription.type);
		resDesc.Heap = GetHeapType(createResourceDescription->heapType);
		resDesc.Flags = GetResourceFlags(createResourceDescription->resourceDescription.usage);
		resDesc.Format = GetPixelFormat(createResourceDescription->resourceDescription.format);
		resDesc.MipCount = Utils::SafeCast<U16>(createResourceDescription->resourceDescription.mipCount);
		if (resDesc.Type == Resource::GenericResourceType::Buffer)
		{
			resDesc.WidthOrBufferSize = createResourceDescription->resourceDescription.size;
			resDesc.HeightOrBufferStride = createResourceDescription->resourceDescription.stride;
		}
		else
		{
			resDesc.WidthOrBufferSize = createResourceDescription->resourceDescription.width;
			resDesc.HeightOrBufferStride = createResourceDescription->resourceDescription.height;
		}
		resDesc.DepthOrArraySize = Utils::SafeCast<U16>(createResourceDescription->resourceDescription.depth);;
		resDesc.InitState = GetState(createResourceDescription->initalState);
		resDesc.InitDataSize = createResourceDescription->initDataSize;
		resDesc.InitData = createResourceDescription->initData;
		ZE_GEN_RES_SET_NAME(resDesc, Utils::ToUTF8(createResourceDescription->name));

		ctx.InternalResources.Add(outTexture->internalIndex, dev, resDesc);

		return FFX_OK;
	}

	FfxErrorCode ExecuteGpuJobs(FfxInterface* backendInterface, FfxCommandList commandList)
	{
		return FFX_OK;
	}
#pragma endregion
#pragma region FFX utility functions
	constexpr FfxBackendInterface& GetFfxCtx(FfxInterface* backendInterface) noexcept
	{
		ZE_ASSERT(backendInterface->scratchBuffer, "Empty FFX backend context!");
		return *((FfxBackendInterface*)backendInterface->scratchBuffer);
	}

	constexpr Device& GetDevice(FfxInterface* backendInterface) noexcept
	{
		ZE_ASSERT(backendInterface->device, "Empty FFX device!");
		return *((Device*)backendInterface->device);
	}

	constexpr Resource::GenericResourceType GetResourceType(FfxResourceType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FFX_RESOURCE_TYPE_BUFFER:
			return Resource::GenericResourceType::Buffer;
		case FFX_RESOURCE_TYPE_TEXTURE1D:
			return Resource::GenericResourceType::Texture1D;
		case FFX_RESOURCE_TYPE_TEXTURE2D:
			return Resource::GenericResourceType::Texture2D;
		case FFX_RESOURCE_TYPE_TEXTURE_CUBE:
			return Resource::GenericResourceType::TextureCube;
		case FFX_RESOURCE_TYPE_TEXTURE3D:
			return Resource::GenericResourceType::Texture3D;
		}
	}

	constexpr Resource::GenericResourceHeap GetHeapType(FfxHeapType type) noexcept
	{
		switch (type)
		{
		case FFX_HEAP_TYPE_DEFAULT:
			return Resource::GenericResourceHeap::GPU;
		default:
			ZE_ENUM_UNHANDLED();
		case FFX_HEAP_TYPE_UPLOAD:
			return Resource::GenericResourceHeap::Upload;
		}
	}

	constexpr Resource::GenericResourceFlags GetResourceFlags(FfxResourceUsage usage) noexcept
	{
		Resource::GenericResourceFlags flags = Resource::GenericResourceFlag::ReadOnly;
		if (usage != FFX_RESOURCE_USAGE_READ_ONLY)
		{
			if (usage & FFX_RESOURCE_USAGE_RENDERTARGET)
				flags |= Resource::GenericResourceFlag::RenderTarget;
			if (usage & FFX_RESOURCE_USAGE_UAV)
				flags |= Resource::GenericResourceFlag::UnorderedAccess;
			if (usage & FFX_RESOURCE_USAGE_DEPTHTARGET)
				flags |= Resource::GenericResourceFlag::DepthBuffer;
			if (usage & FFX_RESOURCE_USAGE_INDIRECT)
				flags |= Resource::GenericResourceFlag::IndirectArguments;
			if (usage & FFX_RESOURCE_USAGE_ARRAYVIEW)
				flags |= Resource::GenericResourceFlag::ArrayView;
		}
		return flags;
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
		case FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT:
			return PixelFormat::R16G16B16A16_Float;
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
		case FFX_SURFACE_FORMAT_R11G11B10_FLOAT:
			return PixelFormat::R11G11B10_Float;
		case FFX_SURFACE_FORMAT_R16G16_FLOAT:
			return PixelFormat::R16G16_Float;
		case FFX_SURFACE_FORMAT_R16G16_UINT:
			return PixelFormat::R16G16_Float;
		case FFX_SURFACE_FORMAT_R16_FLOAT:
			return PixelFormat::R16_Float;
		case FFX_SURFACE_FORMAT_R16_UINT:
			return PixelFormat::R16_UInt;
		case FFX_SURFACE_FORMAT_R16_UNORM:
			return PixelFormat::R16_UNorm;
		case FFX_SURFACE_FORMAT_R16_SNORM:
			return PixelFormat::R16_SNorm;
		case FFX_SURFACE_FORMAT_R8_UNORM:
			return PixelFormat::R8_UNorm;
		case FFX_SURFACE_FORMAT_R8G8_UNORM:
			return PixelFormat::R8G8_UNorm;
		case FFX_SURFACE_FORMAT_R32_FLOAT:
			return PixelFormat::R32_Float;
		}
	}

	constexpr Resource::State GetState(FfxResourceStates state) noexcept
	{
		switch (state)
		{
		case FFX_RESOURCE_STATE_UNORDERED_ACCESS:
			return Resource::State::StateUnorderedAccess;
		case FFX_RESOURCE_STATE_COMPUTE_READ:
			return Resource::State::StateShaderResourceNonPS;
		case FFX_RESOURCE_STATE_PIXEL_READ:
			return Resource::State::StateShaderResourcePS;
		case FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ:
			return Resource::State::StateShaderResourceAll;
		case FFX_RESOURCE_STATE_COPY_SRC:
			return Resource::State::StateCopySource;
		case FFX_RESOURCE_STATE_COPY_DEST:
			return Resource::State::StateCopyDestination;
		case FFX_RESOURCE_STATE_GENERIC_READ:
			return Resource::State::StateGenericRead;
		case FFX_RESOURCE_STATE_INDIRECT_ARGUMENT:
			return Resource::State::StateIndirect;
		default:
		{
			ZE_FAIL("Unhandled resource state!");
			return Resource::State::StateCommon;
		}
		}
	}
#pragma endregion
}