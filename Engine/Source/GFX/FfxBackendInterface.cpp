#include "GFX/FfxBackendInterface.h"
#include "GFX/Pipeline/Barrier.h"
#include "GFX/Resource/Texture/Pack.h"
#include "GFX/Resource/CBuffer.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/FfxEffects.h"
#include "GFX/CommandSignature.h"
#include "Data/Library.h"

// Assert for FFX backend interface pointer
#define ZE_CHECK_FFX_BACKEND() ZE_ASSERT(backendInterface, "Empty FFX backend interface!")

namespace ZE::GFX::FFX
{
	// Custom name for the resource
	struct ResourceName
	{
		wchar_t Name[FFX_RESOURCE_NAME_SIZE];
	};
	// Data about current and starting resource state
	struct ResourceStateInfo
	{
		FfxResourceStates Current;
		FfxResourceStates Initial;
		bool Undefined;
	};
	// Initial data for filling up GPU resources
	struct InitData
	{
		bool IsBuffer;
		Resource::CBuffer Buffer;
		Resource::Texture::Pack Texture;
		U64 LastFrameUsed;
	};
	// Tag for resources registered per frame from outside
	struct DynamicResource
	{
		RID ResID;
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
		IO::DiskManager& Disk;
		Data::Library<S32, FFX::InternalResourceDescription>& InternalBuffers;
		bool& NotifyBuffersChange;
		U32 ContextRefCount = 0;
		PassInfo CurrentPass = {};
		BackendContext* Ctx = nullptr;
	};

	// Interface functions used by FFX SDK backend
	void ffxAssertCallback(const char* message) noexcept;
	FfxUInt32 ffxGetSDKVersion(FfxInterface* backendInterface);
	FfxErrorCode ffxGetEffectGpuMemoryUsage(FfxInterface* backendInterface, FfxUInt32 effectContextId, FfxEffectMemoryUsage* outVramUsage);
	FfxErrorCode ffxCreateBackendContext(FfxInterface* backendInterface, FfxEffect effect,
		FfxEffectBindlessConfig* bindlessConfig, FfxUInt32* effectContextId);
	FfxErrorCode ffxGetDeviceCapabilities(FfxInterface* backendInterface, FfxDeviceCapabilities* deviceCapabilities);
	FfxErrorCode ffxDestroyBackendContext(FfxInterface* backendInterface, FfxUInt32 effectContextId);
	FfxErrorCode ffxCreateResource(FfxInterface* backendInterface,
		const FfxCreateResourceDescription* createResourceDescription,
		FfxUInt32 effectContextId, FfxResourceInternal* outTexture);
	FfxErrorCode ffxDestroyResource(FfxInterface* backendInterface, FfxResourceInternal resource, FfxUInt32 effectContextId);
	FfxResource ffxGetResource(FfxInterface* backendInterface, FfxResourceInternal resource);
	FfxErrorCode ffxRegisterResource(FfxInterface* backendInterface, const FfxResource* inResource,
		FfxUInt32 effectContextId, FfxResourceInternal* outResourceInternal);
	FfxErrorCode ffxRegisterStaticResource(FfxInterface* backendInterface, const FfxStaticResourceDescription* desc, FfxUInt32 effectContextId);
	FfxErrorCode ffxUnregisterResources(FfxInterface* backendInterface, FfxCommandList commandList, FfxUInt32 effectContextId);
	FfxResourceDescription ffxGetResourceDescriptor(FfxInterface* backendInterface, FfxResourceInternal resource);
	FfxErrorCode ffxStageConstantBufferData(FfxInterface* backendInterface, void* data, FfxUInt32 size, FfxConstantBuffer* constantBuffer);
	FfxErrorCode ffxMapResource(FfxInterface* backendInterface, FfxResourceInternal resource, void** ptr);
	FfxErrorCode ffxUnmapResource(FfxInterface* backendInterface, FfxResourceInternal resource);
	FfxErrorCode ffxCreatePipeline(FfxInterface* backendInterface, FfxEffect effect, FfxPass passId,
		uint32_t permutationOptions, const FfxPipelineDescription* desc, FfxUInt32 effectContextId, FfxPipelineState* outPipeline);
	FfxErrorCode ffxDestroyPipeline(FfxInterface* backendInterface, FfxPipelineState* pipeline, FfxUInt32 effectContextId);
	FfxErrorCode ffxScheduleGpuJob(FfxInterface* backendInterface, const FfxGpuJobDescription* job);
	FfxErrorCode ffxExecuteGpuJobs(FfxInterface* backendInterface, FfxCommandList commandList, FfxUInt32 effectContextId);
	FfxErrorCode ffxBreadcrumbsAllocBlock(FfxInterface* backendInterface, U64 blockBytes, FfxBreadcrumbsBlockData* blockData);
	void ffxBreadcrumbsFreeBlock(FfxInterface* backendInterface, FfxBreadcrumbsBlockData* blockData);
	void ffxBreadcrumbsWrite(FfxInterface* backendInterface, FfxCommandList commandList, U32 value, U64 gpuLocation, void* gpuBuffer, bool isBegin);
	void ffxBreadcrumbsPrintDeviceInfo(FfxInterface* backendInterface, FfxAllocationCallbacks* allocs, bool extendedInfo, char** printBuffer, U64* printSize);
	FfxErrorCode ffxGetPermutationBlobByIndex(FfxEffect effectId, FfxPass passId, FfxBindStage bindStage, uint32_t permutationOptions, FfxShaderBlob* outBlob);
	FfxErrorCode ffxSwapChainConfigureFrameGeneration(const FfxFrameGenerationConfig* config);
	void ffxRegisterConstantBufferAllocator(FfxInterface* backendInterface, FfxConstantBufferAllocator fpConstantAllocator) {}

	// Utility functions for working with FFX SDK
	constexpr ResID GetResID(S32 internalIndex) noexcept { ZE_ASSERT(internalIndex, "Invalid FFX resource index"); return static_cast<ResID>(internalIndex - 1); }
	constexpr S32 GetInternalIndex(ResID ffxID) noexcept { return static_cast<S32>(ffxID + 1); }
	// Fix RID for special values to treat it like a pointer
	constexpr void* EncodeRID(RID rid) noexcept { return (void*)static_cast<uintptr_t>(rid == INVALID_RID ? 0 : (rid == BACKBUFFER_RID ? INVALID_RID : rid)); } // No reinterpret_cast to ensure constexpr
	constexpr RID DecodeRID(void* resource) noexcept { return resource ? (((RID)(uintptr_t)(resource)) == INVALID_RID ? BACKBUFFER_RID : (RID)((uintptr_t)resource)) : INVALID_RID; } // No reinterpret_cast to ensure constexpr

	constexpr BackendInterface& GetFfxInterface(FfxInterface* backendInterface) noexcept;
	constexpr BackendContext& GetFfxCtx(BackendInterface& ffxInterface) noexcept;
	constexpr BackendContext& GetFfxCtx(FfxInterface* backendInterface) noexcept;
	constexpr Device& GetDevice(FfxInterface* backendInterface) noexcept;
	constexpr CommandList& GetCommandList(FfxCommandList commandList) noexcept;
	constexpr Pipeline::FrameResourceType GetResourceType(FfxResourceType type) noexcept;
	constexpr Pipeline::FrameResourceFlags GetResourceFlags(FfxResourceUsage usage) noexcept;
	constexpr Pipeline::TextureLayout GetLayout(FfxResourceStates state) noexcept;
	constexpr Resource::Texture::AddressMode GetAddressMode(FfxAddressMode mode) noexcept;
	constexpr Resource::SamplerFilter GetFilter(FfxFilterType filter) noexcept;
	RID GetRID(BackendInterface& ffxInterface, S32 internalIndex) noexcept;
	void AddResourceBarrier(BackendInterface& ffxInterface, S32 internalIndex, FfxResourceStates after) noexcept;
	void FlushBarriers(BackendContext& ctx, CommandList& cl, Pipeline::FrameBuffer& buffers);
	void ExecuteClearJob(BackendInterface& ffxInterface, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxClearFloatJobDescription& job);
	void ExecuteCopyJob(BackendInterface& ffxInterface, Device& dev, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxCopyJobDescription& job);
	void ExecuteComputeJob(BackendInterface& ffxInterface, Device& dev, CommandList& cl, Resource::DynamicCBuffer& dynamicBuffer, Pipeline::FrameBuffer& buffers, const FfxComputeJobDescription& job);
	void ExecuteBarrierJob(BackendInterface& ffxInterface, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxBarrierDescription& job);
	void ExecuteDiscardJob(BackendInterface& ffxInterface, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxDiscardJobDescription& job);

	FfxResource GetResource(Pipeline::FrameBuffer& buffers, RID rid, FfxResourceStates state) noexcept
	{
		FfxResource desc = {};
		desc.resource = EncodeRID(rid);
		if (rid != INVALID_RID)
		{
			if (buffers.IsCubeTexture(rid))
				desc.description.type = FFX_RESOURCE_TYPE_TEXTURE_CUBE;
			else if (buffers.IsTexture1D(rid))
				desc.description.type = FFX_RESOURCE_TYPE_TEXTURE1D;
			else if (buffers.IsTexture3D(rid))
				desc.description.type = FFX_RESOURCE_TYPE_TEXTURE3D;
			else if (buffers.IsBuffer(rid))
				desc.description.type = FFX_RESOURCE_TYPE_BUFFER;
			else
				desc.description.type = FFX_RESOURCE_TYPE_TEXTURE2D;

			desc.description.format = GetSurfaceFormat(buffers.GetFormat(rid));
			UInt2 sizes = buffers.GetDimmensions(rid);
			if (desc.description.type == FFX_RESOURCE_TYPE_BUFFER)
			{
				desc.description.size = sizes.X;
				desc.description.stride = sizes.Y;
				desc.description.alignment = 0;
			}
			else
			{
				desc.description.width = sizes.X;
				desc.description.height = sizes.Y;
				desc.description.depth = buffers.GetArraySize(rid);
			}
			desc.description.mipCount = buffers.GetMipCount(rid);
			desc.description.flags = FFX_RESOURCE_FLAGS_NONE;
			desc.description.usage = FFX_RESOURCE_USAGE_READ_ONLY;
			if (buffers.IsUAV(rid))
				desc.description.usage = static_cast<FfxResourceUsage>(desc.description.usage | FFX_RESOURCE_USAGE_UAV);
			if (buffers.IsArrayView(rid))
				desc.description.usage = static_cast<FfxResourceUsage>(desc.description.usage | FFX_RESOURCE_USAGE_ARRAYVIEW);
			desc.state = state;
		}
		return desc;
	}

	FfxInterface GetInterface(Device& dev, ChainPool<Resource::DynamicCBuffer>& dynamicBuffers,
		Pipeline::FrameBuffer& frameBuffer, IO::DiskManager& disk,
		Data::Library<S32, FFX::InternalResourceDescription>& internalBuffers,
		bool& notifyBuffersChange) noexcept
	{
		FfxInterface backendInterface = {};
		backendInterface.fpGetSDKVersion = ffxGetSDKVersion;
		backendInterface.fpGetEffectGpuMemoryUsage = ffxGetEffectGpuMemoryUsage;
		backendInterface.fpCreateBackendContext = ffxCreateBackendContext;
		backendInterface.fpGetDeviceCapabilities = ffxGetDeviceCapabilities;
		backendInterface.fpDestroyBackendContext = ffxDestroyBackendContext;
		backendInterface.fpCreateResource = ffxCreateResource;
		backendInterface.fpDestroyResource = ffxDestroyResource;
		backendInterface.fpGetResource = ffxGetResource;
		backendInterface.fpRegisterResource = ffxRegisterResource;
		backendInterface.fpRegisterStaticResource = ffxRegisterStaticResource;
		backendInterface.fpUnregisterResources = ffxUnregisterResources;
		backendInterface.fpGetResourceDescription = ffxGetResourceDescriptor;
		backendInterface.fpMapResource = ffxMapResource;
		backendInterface.fpUnmapResource = ffxUnmapResource;
		backendInterface.fpStageConstantBufferDataFunc = ffxStageConstantBufferData;
		backendInterface.fpCreatePipeline = ffxCreatePipeline;
		backendInterface.fpDestroyPipeline = ffxDestroyPipeline;
		backendInterface.fpScheduleGpuJob = ffxScheduleGpuJob;
		backendInterface.fpExecuteGpuJobs = ffxExecuteGpuJobs;

		backendInterface.fpBreadcrumbsAllocBlock = ffxBreadcrumbsAllocBlock;
		backendInterface.fpBreadcrumbsFreeBlock = ffxBreadcrumbsFreeBlock;
		backendInterface.fpBreadcrumbsWrite = ffxBreadcrumbsWrite;
		backendInterface.fpBreadcrumbsPrintDeviceInfo = ffxBreadcrumbsPrintDeviceInfo;

		backendInterface.fpGetPermutationBlobByIndex = ffxGetPermutationBlobByIndex;
		backendInterface.fpSwapChainConfigureFrameGeneration = ffxSwapChainConfigureFrameGeneration;
		backendInterface.fpRegisterConstantBufferAllocator = ffxRegisterConstantBufferAllocator;

		// Setup all custom backend memory
		backendInterface.scratchBufferSize = sizeof(BackendInterface);
		backendInterface.scratchBuffer = new BackendInterface{ dynamicBuffers, frameBuffer, disk, internalBuffers, notifyBuffersChange };
		backendInterface.device = &dev;

		// Set assert printing
		ffxAssertSetPrintingCallback(ffxAssertCallback);
		return backendInterface;
	}

	void SetCurrentPass(FfxInterface& backendInterface, const PassInfo* info) noexcept
	{
		GetFfxInterface(&backendInterface).CurrentPass = info ? *info : PassInfo{};
	}

	void DestroyInterface(FfxInterface& backendInterface) noexcept
	{
		if (backendInterface.scratchBuffer)
		{
			delete reinterpret_cast<BackendInterface*>(backendInterface.scratchBuffer);
			backendInterface.scratchBuffer = nullptr;
		}
	}

#pragma region FFX backend functions
	void ffxAssertCallback(const char* message) noexcept
	{
		ZE_FAIL(message);
	}

	FfxUInt32 ffxGetSDKVersion(FfxInterface* backendInterface)
	{
		return FFX_SDK_MAKE_VERSION(FFX_SDK_VERSION_MAJOR, FFX_SDK_VERSION_MINOR, FFX_SDK_VERSION_PATCH);
	}

	FfxErrorCode ffxGetEffectGpuMemoryUsage(FfxInterface* backendInterface, FfxUInt32 effectContextId, FfxEffectMemoryUsage* outVramUsage)
	{
		*outVramUsage = GetFfxCtx(backendInterface).EffectMemoryUsage.Get(static_cast<FfxEffect>(effectContextId));
		return FFX_OK;
	}

	FfxErrorCode ffxCreateBackendContext(FfxInterface* backendInterface, FfxEffect effect,
		FfxEffectBindlessConfig* bindlessConfig, FfxUInt32* effectContextId)
	{
		BackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		if (effectContextId)
			*effectContextId = effect;

		if (ffxInterface.ContextRefCount++ == 0)
		{
			U8* buffer = new U8[sizeof(BackendContext)];
			std::memset(buffer, 0, sizeof(BackendContext));
			reinterpret_cast<BackendInterface*>(backendInterface->scratchBuffer)->Ctx = reinterpret_cast<BackendContext*>(buffer);

			new(reinterpret_cast<BackendContext*>(buffer)) BackendContext;
		}

		BackendContext& ctx = GetFfxCtx(ffxInterface);
		if (!ctx.EffectMemoryUsage.Contains(effect))
			ctx.EffectMemoryUsage.Add(effect, { 0, 0 });

		return FFX_OK;
	}

	FfxErrorCode ffxGetDeviceCapabilities(FfxInterface* backendInterface, FfxDeviceCapabilities* deviceCapabilities)
	{
		ZE_ASSERT(deviceCapabilities, "Empty FFX device capabilities!");

		Device& dev = GetDevice(backendInterface);
		switch (dev.GetMaxShaderModel())
		{
		case ShaderModel::V5_0:
		ZE_WARNING("No option to specify lower shader model in FFX SDK than 5.1 so in case of older APIs assume 5.1");
		[[fallthrough]];
		case ShaderModel::V5_1:
		deviceCapabilities->maximumSupportedShaderModel = FFX_SHADER_MODEL_5_1;
		break;
		case ShaderModel::V6_0:
		deviceCapabilities->maximumSupportedShaderModel = FFX_SHADER_MODEL_6_0;
		break;
		case ShaderModel::V6_1:
		deviceCapabilities->maximumSupportedShaderModel = FFX_SHADER_MODEL_6_1;
		break;
		case ShaderModel::V6_2:
		deviceCapabilities->maximumSupportedShaderModel = FFX_SHADER_MODEL_6_2;
		break;
		case ShaderModel::V6_3:
		deviceCapabilities->maximumSupportedShaderModel = FFX_SHADER_MODEL_6_3;
		break;
		case ShaderModel::V6_4:
		deviceCapabilities->maximumSupportedShaderModel = FFX_SHADER_MODEL_6_5;
		break;
		case ShaderModel::V6_5:
		deviceCapabilities->maximumSupportedShaderModel = FFX_SHADER_MODEL_6_5;
		break;
		case ShaderModel::V6_6:
		deviceCapabilities->maximumSupportedShaderModel = FFX_SHADER_MODEL_6_6;
		break;
		case ShaderModel::V6_7:
		case ShaderModel::V6_8:
		case ShaderModel::V6_9:
		deviceCapabilities->maximumSupportedShaderModel = FFX_SHADER_MODEL_6_7;
		break;
		}

		auto minMax = dev.GetWaveLaneCountRange();
		deviceCapabilities->waveLaneCountMin = minMax.first;
		deviceCapabilities->waveLaneCountMax = minMax.second;
		deviceCapabilities->fp16Supported = dev.IsShaderFloat16Supported();
		deviceCapabilities->raytracingSupported = Settings::RayTracingTier != GFX::RayTracingTier::None;
		deviceCapabilities->deviceCoherentMemorySupported = dev.IsCoherentMemorySupported();
		deviceCapabilities->dedicatedAllocationSupported = dev.IsDedicatedAllocSupported();
		deviceCapabilities->bufferMarkerSupported = dev.IsBufferMarkersSupported();
		deviceCapabilities->extendedSynchronizationSupported = dev.IsExtendedSynchronizationSupported();
		deviceCapabilities->shaderStorageBufferArrayNonUniformIndexing = dev.IsUavNonUniformIndexing();

		return FFX_OK;
	}

	FfxErrorCode ffxDestroyBackendContext(FfxInterface* backendInterface, FfxUInt32 effectContextId)
	{
		BackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		if (--ffxInterface.ContextRefCount == 0)
		{
			Device& dev = GetDevice(backendInterface);
			BackendContext& ctx = GetFfxCtx(backendInterface);

			if (ffxInterface.InternalBuffers.Size())
			{
				ffxInterface.InternalBuffers.Clear();
				ffxInterface.NotifyBuffersChange = true;
			}

			// Free all remaining init resources
			for (ResID ffxId : ctx.Resources.view<InitData>())
			{
				InitData& initData = ctx.Resources.get<InitData>(ffxId);
				initData.IsBuffer ? initData.Buffer.Free(dev) : initData.Texture.Free(dev);
			}
			ctx.Resources.clear();

			ctx.Pipelines.Transform([&dev](Resource::PipelineStateCompute& pipeline) { pipeline.Free(dev); });
			ctx.Bindings.Transform([&dev](Binding::Schema& schema) { schema.Free(dev); });
			ctx.CommandSignatures.Transform([&dev](CommandSignature& signature) { signature.Free(dev); });

			ctx.~BackendContext();
			delete[] reinterpret_cast<U8*>(ffxInterface.Ctx);
			ffxInterface.Ctx = nullptr;
		}
		return FFX_OK;
	}

	FfxErrorCode ffxCreateResource(FfxInterface* backendInterface,
		const FfxCreateResourceDescription* createResourceDescription,
		FfxUInt32 effectContextId, FfxResourceInternal* outTexture)
	{
		ZE_ASSERT(createResourceDescription, "Empty FFX resource description");
		ZE_ASSERT(outTexture, "Empty FFX out texture!");

		// FFX_RESOURCE_FLAGS_UNDEFINED -> not used as handling undefined layout comes naturaly here

		BackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		BackendContext& ctx = GetFfxCtx(ffxInterface);
		Device& dev = GetDevice(backendInterface);

		ResID ffxID = ctx.Resources.create();
		outTexture->internalIndex = GetInternalIndex(ffxID);

		ZE_ASSERT(createResourceDescription->heapType == FFX_HEAP_TYPE_DEFAULT, "Need to account for new type of heap!");
		ZE_ASSERT(!ffxInterface.InternalBuffers.Contains(outTexture->internalIndex), "Resource has been already created!");

		Pipeline::FrameResourceDesc resDesc = {};
		resDesc.Format = GetPixelFormat(createResourceDescription->resourceDescription.format);
		if (createResourceDescription->resourceDescription.type == FFX_RESOURCE_TYPE_BUFFER)
		{
			const U32 formatSize = static_cast<U32>(Utils::GetFormatSize(resDesc.Format));
			resDesc.Sizes.X = Math::AlignUp(createResourceDescription->resourceDescription.size, formatSize);
			resDesc.Sizes.Y = Math::AlignUp(createResourceDescription->resourceDescription.stride, formatSize);
			resDesc.DepthOrArraySize = 1;
		}
		else
		{
			resDesc.Sizes.X = createResourceDescription->resourceDescription.width;
			resDesc.Sizes.Y = createResourceDescription->resourceDescription.height;
			resDesc.DepthOrArraySize = Utils::SafeCast<U16>(createResourceDescription->resourceDescription.depth);
		}

		resDesc.Flags = GetResourceFlags(createResourceDescription->resourceDescription.usage);
		// Allow for aliasing with other resources
		if ((createResourceDescription->resourceDescription.flags & FFX_RESOURCE_FLAGS_ALIASABLE) == 0)
			resDesc.Flags |= Pipeline::FrameResourceFlag::Temporal;

		resDesc.ClearColor = ColorF4{};
		resDesc.ClearDepth = 0.0f;
		resDesc.ClearStencil = 0;
		resDesc.MipLevels = Utils::SafeCast<U16>(createResourceDescription->resourceDescription.mipCount);
		resDesc.Type = GetResourceType(createResourceDescription->resourceDescription.type);
		ZE_FRAME_RES_SET_NAME(resDesc, Utils::ToUTF8(createResourceDescription->name));

		if (createResourceDescription->initData.type == FFX_RESOURCE_INIT_DATA_TYPE_BUFFER
			|| createResourceDescription->initData.type == FFX_RESOURCE_INIT_DATA_TYPE_VALUE)
		{
			const bool isValue = createResourceDescription->initData.type == FFX_RESOURCE_INIT_DATA_TYPE_VALUE;
			InitData& initData = ctx.Resources.emplace<InitData>(ffxID,
				createResourceDescription->resourceDescription.type == FFX_RESOURCE_TYPE_BUFFER);
			initData.LastFrameUsed = UINT64_MAX; // Indication that it shouldn't be removed
			if (initData.IsBuffer)
			{
				std::vector<U8> initValue;
				Resource::CBufferData data = {};
				data.Bytes = Math::AlignUp(Utils::SafeCast<U32>(createResourceDescription->initData.size), static_cast<U32>(Utils::GetFormatSize(resDesc.Format)));
				if (isValue)
				{
					initValue.resize(data.Bytes, createResourceDescription->initData.value);
					data.DataStatic = initValue.data();
				}
				else
					data.DataStatic = createResourceDescription->initData.buffer;
				initData.Buffer.Init(dev, ffxInterface.Disk, data);
			}
			else
			{
				// Handling of textures
				std::vector<Surface> surfaces;
				surfaces.emplace_back(resDesc.Sizes.X, resDesc.Sizes.Y, resDesc.DepthOrArraySize, resDesc.MipLevels,
					static_cast<U16>(1U), resDesc.Format, false, isValue ? &createResourceDescription->initData.value : createResourceDescription->initData.buffer);
				Resource::Texture::PackDesc packDesc = {};
				packDesc.Options = Resource::Texture::PackOption::CopySource;

				Resource::Texture::Type texType = Resource::Texture::Type::Tex2D;
				switch (createResourceDescription->resourceDescription.type)
				{
				case FFX_RESOURCE_TYPE_TEXTURE1D:
				texType = Resource::Texture::Type::Tex1D;
				break;
				default:
				ZE_ENUM_UNHANDLED();
				case FFX_RESOURCE_TYPE_TEXTURE2D:
				break;
				case FFX_RESOURCE_TYPE_TEXTURE_CUBE:
				texType = Resource::Texture::Type::Cube;
				break;
				case FFX_RESOURCE_TYPE_TEXTURE3D:
				texType = Resource::Texture::Type::Tex3D;
				break;
				}
				packDesc.AddTexture(texType, std::move(surfaces));
				ZE_TEXTURE_SET_NAME(packDesc, Utils::ToUTF8(createResourceDescription->name) + "_INIT_DATA");
				initData.Texture.Init(dev, ffxInterface.Disk, packDesc);
			}
			DiskStatusHandle diskStatus = ffxInterface.Disk.SetGPUUploadWaitPoint();
			ffxInterface.Disk.StartUploadGPU();

			CommandList barrierCL;
			bool workPending = ffxInterface.Disk.IsGPUWorkPending(diskStatus);
			if (workPending)
			{
				barrierCL.Init(dev);
				barrierCL.Open(dev);
			}
			bool status = ffxInterface.Disk.WaitForUploadGPU(dev, barrierCL, diskStatus);
			ZE_ASSERT(status, "Error uploading initial FFX resource data!");
			if (workPending)
			{
				barrierCL.Close(dev);
				dev.ExecuteMain(barrierCL);
				dev.WaitMain(dev.SetMainFence());
				barrierCL.Free(dev);
			}
			if (!status)
			{
				ctx.Resources.destroy(ffxID);
				outTexture->internalIndex = 0;
				return FFX_ERROR_BACKEND_API_ERROR;
			}

			// Special type of job, meaning to initialize resource data
			FfxGpuJobDescription copyJob = {};
			copyJob.jobType = FFX_GPU_JOB_COPY;
			copyJob.copyJobDescriptor.src = copyJob.copyJobDescriptor.dst = *outTexture;
			ffxScheduleGpuJob(backendInterface, &copyJob);
		}

		ffxInterface.InternalBuffers.Add(outTexture->internalIndex, resDesc, INVALID_RID, effectContextId);
		ffxInterface.NotifyBuffersChange = true;

		ctx.Resources.emplace<ResourceStateInfo>(ffxID, createResourceDescription->initialState, createResourceDescription->initialState, true);
		ctx.Resources.emplace<FfxResourceDescription>(ffxID, createResourceDescription->resourceDescription);
#if _ZE_DEBUG_GFX_NAMES
		if (createResourceDescription->name)
			wcscpy_s(ctx.Resources.emplace<ResourceName>(ffxID).Name, createResourceDescription->name);
#endif
		return FFX_OK;
	}

	FfxErrorCode ffxDestroyResource(FfxInterface* backendInterface, FfxResourceInternal resource, FfxUInt32 effectContextId)
	{
		if (resource.internalIndex)
		{
			BackendInterface& ffxInterface = GetFfxInterface(backendInterface);
			BackendContext& ctx = GetFfxCtx(ffxInterface);
			ResID id = GetResID(resource.internalIndex);

			// Due to different handling of internal resources sometimes there can be request to delete resource that is not present (eg. FSR2 copy resources)
			if (ctx.Resources.valid(id))
			{
				Device& dev = GetDevice(backendInterface);
				ZE_ASSERT(ffxInterface.InternalBuffers.Contains(resource.internalIndex), "Resource has not been properly created!");

				InitData* initData = ctx.Resources.try_get<InitData>(id);
				if (initData)
					initData->IsBuffer ? initData->Buffer.Free(dev) : initData->Texture.Free(dev);
				ffxInterface.InternalBuffers.Remove(resource.internalIndex);
				ffxInterface.NotifyBuffersChange = true;
				ctx.Resources.destroy(id);
			}
		}
		return FFX_OK;
	}

	FfxResource ffxGetResource(FfxInterface* backendInterface, FfxResourceInternal resource)
	{
		BackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		BackendContext& ctx = GetFfxCtx(ffxInterface);
		ResID ffxId = GetResID(resource.internalIndex);

		FfxResource res = {};
		res.resource = EncodeRID(GetRID(ffxInterface, resource.internalIndex));
		res.state = ctx.Resources.get<ResourceStateInfo>(ffxId).Current;
		res.description = ffxGetResourceDescriptor(backendInterface, resource);
#if _ZE_DEBUG_GFX_NAMES
		if (ResourceName* name = ctx.Resources.try_get<ResourceName>(ffxId))
			wcscpy_s(res.name, name->Name);
#endif
		return res;
	}

	FfxErrorCode ffxRegisterResource(FfxInterface* backendInterface, const FfxResource* inResource,
		FfxUInt32 effectContextId, FfxResourceInternal* outResourceInternal)
	{
		ZE_ASSERT(inResource, "Empty FFX input resource!");
		ZE_ASSERT(outResourceInternal, "Empty FFX out resource!");

		if (inResource->resource)
		{
			BackendContext& ctx = GetFfxCtx(backendInterface);
			ResID ffxId = ctx.Resources.create();
			outResourceInternal->internalIndex = GetInternalIndex(ffxId);

			ctx.Resources.emplace<ResourceStateInfo>(ffxId, inResource->state, inResource->state, false);
			ctx.Resources.emplace<FfxResourceDescription>(ffxId, inResource->description);
#if _ZE_DEBUG_GFX_NAMES
			if (inResource->name)
				wcscpy_s(ctx.Resources.emplace<ResourceName>(ffxId).Name, inResource->name);
#endif
			// Tag as dynamic per-frame resource
			ctx.Resources.emplace<DynamicResource>(ffxId, DecodeRID(inResource->resource));
		}
		else
			outResourceInternal->internalIndex = 0;
		return FFX_OK;
	}

	FfxErrorCode ffxRegisterStaticResource(FfxInterface* backendInterface, const FfxStaticResourceDescription* desc, FfxUInt32 effectContextId)
	{
		ZE_FAIL("Static resources are not yet supported!");
		return FFX_ERROR_INCOMPLETE_INTERFACE;
	}

	FfxErrorCode ffxUnregisterResources(FfxInterface* backendInterface, FfxCommandList commandList, FfxUInt32 effectContextId)
	{
		BackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		BackendContext& ctx = GetFfxCtx(ffxInterface);

		// Walk back all the resources that don't belong to FFX and reset them to their initial state
		for (ResID res : ctx.Resources.view<DynamicResource>())
			AddResourceBarrier(ffxInterface, GetInternalIndex(res), ctx.Resources.get<ResourceStateInfo>(res).Initial);
		FlushBarriers(ctx, GetCommandList(commandList), GetFfxInterface(backendInterface).Buffers);

		// Clear dynamic resources
		ctx.Resources.destroy(ctx.Resources.view<DynamicResource>().begin(), ctx.Resources.view<DynamicResource>().end());
		return FFX_OK;
	}

	FfxResourceDescription ffxGetResourceDescriptor(FfxInterface* backendInterface, FfxResourceInternal resource)
	{
		return GetFfxCtx(backendInterface).Resources.get<FfxResourceDescription>(GetResID(resource.internalIndex));
	}

	FfxErrorCode ffxStageConstantBufferData(FfxInterface* backendInterface, void* data, FfxUInt32 size, FfxConstantBuffer* constantBuffer)
	{
		if (data && constantBuffer)
		{
			Device& dev = GetDevice(backendInterface);
			auto alloc = GetFfxInterface(backendInterface).DynamicBuffers.Get().Alloc(dev, data, size);

			constantBuffer->data = reinterpret_cast<U32*>(alloc.Block);
			constantBuffer->num32BitEntries = alloc.Offset;
			return FFX_OK;
		}
		else
			return FFX_ERROR_INVALID_POINTER;
	}

	FfxErrorCode ffxMapResource(FfxInterface* backendInterface, FfxResourceInternal resource, void** ptr)
	{
		BackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		ffxInterface.Buffers.MapResource(GetDevice(backendInterface), GetRID(ffxInterface, resource.internalIndex), ptr);
		return FFX_OK;
	}

	FfxErrorCode ffxUnmapResource(FfxInterface* backendInterface, FfxResourceInternal resource)
	{
		BackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		ffxInterface.Buffers.UnmapResource(GetRID(ffxInterface, resource.internalIndex));
		return FFX_OK;
	}

	FfxErrorCode ffxCreatePipeline(FfxInterface* backendInterface, FfxEffect effect, FfxPass passId,
		uint32_t permutationOptions, const FfxPipelineDescription* desc, FfxUInt32 effectContextId, FfxPipelineState* outPipeline)
	{
		ZE_ASSERT(desc, "Empty FFX pipeline desc!");
		ZE_ASSERT(outPipeline, "Empty FFX pipeline state!");
		ZE_ASSERT(desc->stage == FFX_BIND_COMPUTE_SHADER_STAGE || desc->stage == 0, "Pipeline not for the compute shader!");

		Device& dev = GetDevice(backendInterface);
		BackendContext& ctx = GetFfxCtx(backendInterface);
		const U64 id = FFX::GetPipelineID(effect, passId, permutationOptions);

		FfxShaderBlob shaderBlob = {};
		if (!ctx.Pipelines.Contains(id))
		{
			// Fill out for every shader that is created when needed by effect
			Resource::Shader shader;
			const FfxErrorCode code = GetShaderInfo(&dev, effect, passId, permutationOptions, shaderBlob, &shader);
			if (code != FFX_OK)
				return code;

			// Create binding description
			Binding::SchemaDesc schemaDesc = {};
			schemaDesc.Options = Binding::SchemaOption::NoVertexBuffer;

			// Setup samplers
			for (U32 i = 0; i < desc->samplerCount; ++i)
			{
				// Stage member is currently unused
				ZE_ASSERT(desc->samplers[i].stage == FFX_BIND_COMPUTE_SHADER_STAGE, "Binding not for the compute shader!");
				schemaDesc.AddSampler(
					{
						GetFilter(desc->samplers[i].filter),
					{
						GetAddressMode(desc->samplers[i].addressModeU),
						GetAddressMode(desc->samplers[i].addressModeV),
						GetAddressMode(desc->samplers[i].addressModeW)
					},
					0.0f,
					16,
					Resource::CompareMethod::Never,
					Resource::Texture::EdgeColor::TransparentBlack,
					0.0f,
					FLT_MAX,
					i
					});
			}

			// Set bindings (UAV, SRV, CBV)
			U8 rangeOffset = 0;
			ZE_ASSERT(rangeOffset + shaderBlob.uavBufferCount <= UINT8_MAX, "Buffers outside possible range!");
			for (U32 i = 0; i < shaderBlob.uavBufferCount; ++i)
			{
				schemaDesc.AddRange({ shaderBlob.boundUAVBufferCounts[i], shaderBlob.boundUAVBuffers[i], rangeOffset,
					Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack });
				++rangeOffset;
			}

			ZE_ASSERT(rangeOffset + shaderBlob.uavTextureCount <= UINT8_MAX, "Buffers outside possible range!");
			for (U32 i = 0; i < shaderBlob.uavTextureCount; ++i)
			{
				schemaDesc.AddRange({ shaderBlob.boundUAVTextureCounts[i], shaderBlob.boundUAVTextures[i], rangeOffset,
					Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack });
				++rangeOffset;
			}

			ZE_ASSERT(rangeOffset + shaderBlob.srvBufferCount <= UINT8_MAX, "Buffers outside possible range!");
			for (U32 i = 0; i < shaderBlob.srvBufferCount; ++i)
			{
				schemaDesc.AddRange({ shaderBlob.boundSRVBufferCounts[i], shaderBlob.boundSRVBuffers[i], rangeOffset,
					Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
				++rangeOffset;
			}

			ZE_ASSERT(rangeOffset + shaderBlob.srvTextureCount <= UINT8_MAX, "Buffers outside possible range!");
			for (U32 i = 0; i < shaderBlob.srvTextureCount; ++i)
			{
				schemaDesc.AddRange({ shaderBlob.boundSRVTextureCounts[i], shaderBlob.boundSRVTextures[i], rangeOffset,
					Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
				++rangeOffset;
			}

			for (U32 i = 0; i < shaderBlob.cbvCount; ++i)
			{
				schemaDesc.AddRange({ shaderBlob.boundConstantBufferCounts[i], shaderBlob.boundConstantBuffers[i], rangeOffset,
					Resource::ShaderType::Compute, Binding::RangeFlag::CBV });
				rangeOffset += Utils::SafeCast<U8>(shaderBlob.boundConstantBufferCounts[i]);
			}
			// TODO: cache binding based on input parameters
			if (!ctx.Bindings.Contains(id))
				ctx.Bindings.Add(id, dev, schemaDesc);
			if (!ctx.BindingsReferences.Contains(id))
				ctx.BindingsReferences.Add(id, 0ULL);

			// Create pipeline
			if (!ctx.Pipelines.Contains(id))
				ctx.Pipelines.Add(id, dev, shader, ctx.Bindings.Get(id));
			if (!ctx.PipelinesReferences.Contains(id))
				ctx.PipelinesReferences.Add(id, 0ULL);
			shader.Free(dev);
		}
		else
		{
			// Shader already loaded but resource info still needed
			const FfxErrorCode code = GetShaderInfo(nullptr, effect, passId, permutationOptions, shaderBlob, nullptr);
			if (code != FFX_OK)
				return code;
		}
		outPipeline->rootSignature = reinterpret_cast<FfxRootSignature>(id);
		outPipeline->pipeline = reinterpret_cast<FfxPipeline>(id);

		++ctx.BindingsReferences.Get(id);
		++ctx.PipelinesReferences.Get(id);

		// Fill out bindings
		outPipeline->uavTextureCount = shaderBlob.uavTextureCount;
		outPipeline->srvTextureCount = shaderBlob.srvTextureCount;
		outPipeline->srvBufferCount = shaderBlob.srvBufferCount;
		outPipeline->uavBufferCount = shaderBlob.uavBufferCount;
		outPipeline->constCount = shaderBlob.cbvCount;

		for (U32 i = 0; i < shaderBlob.uavTextureCount; ++i)
		{
			outPipeline->uavTextureBindings[i].slotIndex = shaderBlob.boundUAVTextures[i];
#if _ZE_DEBUG_GFX_NAMES
			wcscpy_s(outPipeline->uavTextureBindings[i].name, Utils::ToUTF16(shaderBlob.boundUAVTextureNames[i]).c_str());
#endif
		}
		for (U32 i = 0; i < shaderBlob.srvTextureCount; ++i)
		{
			outPipeline->srvTextureBindings[i].slotIndex = shaderBlob.boundSRVTextures[i];
#if _ZE_DEBUG_GFX_NAMES
			wcscpy_s(outPipeline->srvTextureBindings[i].name, Utils::ToUTF16(shaderBlob.boundSRVTextureNames[i]).c_str());
#endif
		}
		for (U32 i = 0; i < shaderBlob.srvBufferCount; ++i)
		{
			outPipeline->srvBufferBindings[i].slotIndex = shaderBlob.boundSRVBuffers[i];
#if _ZE_DEBUG_GFX_NAMES
			wcscpy_s(outPipeline->srvBufferBindings[i].name, Utils::ToUTF16(shaderBlob.boundSRVBufferNames[i]).c_str());
#endif
		}
		for (U32 i = 0; i < shaderBlob.uavBufferCount; ++i)
		{
			outPipeline->uavBufferBindings[i].slotIndex = shaderBlob.boundUAVBuffers[i];
#if _ZE_DEBUG_GFX_NAMES
			wcscpy_s(outPipeline->uavBufferBindings[i].name, Utils::ToUTF16(shaderBlob.boundUAVBufferNames[i]).c_str());
#endif
		}
		for (U32 i = 0; i < shaderBlob.cbvCount; ++i)
		{
			outPipeline->constantBufferBindings[i].slotIndex = shaderBlob.boundConstantBuffers[i];
#if _ZE_DEBUG_GFX_NAMES
			wcscpy_s(outPipeline->constantBufferBindings[i].name, Utils::ToUTF16(shaderBlob.boundConstantBufferNames[i]).c_str());
#endif
		}

		// Only set the command signature if this is setup as an indirect workload
		if (desc->indirectWorkload)
		{
			if (!ctx.CommandSignatures.Contains(IndirectCommandType::Dispatch))
			{
				ctx.CommandSignatures.Add(IndirectCommandType::Dispatch, dev, IndirectCommandType::Dispatch);
				ctx.CommandSignaturesReferences.Add(IndirectCommandType::Dispatch, 0ULL);
			}
			outPipeline->cmdSignature = reinterpret_cast<FfxCommandSignature>(IndirectCommandType::Dispatch);
			++ctx.CommandSignaturesReferences.Get(IndirectCommandType::Dispatch);
		}
		else
			outPipeline->cmdSignature = nullptr;

		return FFX_OK;
	}

	FfxErrorCode ffxDestroyPipeline(FfxInterface* backendInterface, FfxPipelineState* pipeline, FfxUInt32 effectContextId)
	{
		if (pipeline)
		{
			Device& dev = GetDevice(backendInterface);
			BackendContext& ctx = GetFfxCtx(backendInterface);

			if (pipeline->rootSignature)
			{
				const U64 id = reinterpret_cast<U64>(pipeline->rootSignature);
				ZE_ASSERT(ctx.BindingsReferences.Get(id) > 0, "Incorrect FFX binding reference count!");
				if (--ctx.BindingsReferences.Get(id) == 0)
				{
					ctx.Bindings.Get(id).Free(dev);
					ctx.Bindings.Remove(id);
				}
				pipeline->rootSignature = nullptr;
			}
			if (pipeline->cmdSignature)
			{
				const IndirectCommandType id = static_cast<IndirectCommandType>(reinterpret_cast<U64>(pipeline->cmdSignature));
				ZE_ASSERT(ctx.CommandSignaturesReferences.Get(id) > 0, "Incorrect FFX command signature reference count!");
				if (--ctx.CommandSignaturesReferences.Get(id) == 0)
				{
					ctx.CommandSignatures.Get(id).Free(dev);
					ctx.CommandSignatures.Remove(id);
				}
				pipeline->cmdSignature = nullptr;
			}
			if (pipeline->pipeline)
			{
				const U64 id = reinterpret_cast<U64>(pipeline->pipeline);
				ZE_ASSERT(ctx.PipelinesReferences.Get(id) > 0, "Incorrect FFX pipeline reference count!");
				if (--ctx.PipelinesReferences.Get(id) == 0)
				{
					ctx.Pipelines.Get(id).Free(dev);
					ctx.Pipelines.Remove(id);
				}
				pipeline->pipeline = nullptr;
			}
		}
		return FFX_OK;
	}

	FfxErrorCode ffxScheduleGpuJob(FfxInterface* backendInterface, const FfxGpuJobDescription* job)
	{
		ZE_ASSERT(job, "Empty FFX gpu job!");

		GetFfxCtx(backendInterface).Jobs.emplace_back(*job);
		return FFX_OK;
	}

	FfxErrorCode ffxExecuteGpuJobs(FfxInterface* backendInterface, FfxCommandList commandList, FfxUInt32 effectContextId)
	{
		BackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		BackendContext& ctx = GetFfxCtx(ffxInterface);
		Device& dev = GetDevice(backendInterface);
		Resource::DynamicCBuffer& dynamicBuffer = ffxInterface.DynamicBuffers.Get();
		CommandList& cl = GetCommandList(commandList);

		for (const FfxGpuJobDescription& job : ctx.Jobs)
		{
			switch (job.jobType)
			{
			case FFX_GPU_JOB_CLEAR_FLOAT:
			ExecuteClearJob(ffxInterface, cl, ffxInterface.Buffers, job.clearJobDescriptor);
			break;
			case FFX_GPU_JOB_COPY:
			ExecuteCopyJob(ffxInterface, dev, cl, ffxInterface.Buffers, job.copyJobDescriptor);
			break;
			case FFX_GPU_JOB_COMPUTE:
			ExecuteComputeJob(ffxInterface, dev, cl, dynamicBuffer, ffxInterface.Buffers, job.computeJobDescriptor);
			break;
			case FFX_GPU_JOB_BARRIER:
			ExecuteBarrierJob(ffxInterface, cl, ffxInterface.Buffers, job.barrierDescriptor);
			break;
			case FFX_GPU_JOB_DISCARD:
			ExecuteDiscardJob(ffxInterface, cl, ffxInterface.Buffers, job.discardJobDescriptor);
			break;
			default:
			ZE_FAIL("Unknown FFX GPU job!");
			break;
			}
		}
		ctx.Jobs.clear();

		for (ResID ffxId : ctx.Resources.view<InitData>())
		{
			InitData& initData = ctx.Resources.get<InitData>(ffxId);
			if (initData.LastFrameUsed + Settings::GetBackbufferCount() < Settings::GetFrameIndex())
			{
				initData.IsBuffer ? initData.Buffer.Free(dev) : initData.Texture.Free(dev);
				ctx.Resources.remove<InitData>(ffxId);
			}
		}

		return FFX_OK;
	}

	FfxErrorCode ffxBreadcrumbsAllocBlock(FfxInterface* backendInterface, U64 blockBytes, FfxBreadcrumbsBlockData* blockData)
	{
		ZE_ASSERT(blockData != nullptr, "Empty FFX Breadcrumbs block data!");

		*blockData = GetDevice(backendInterface).AllocBreadcrumbsBlock(blockBytes);
		return FFX_OK;
	}

	void ffxBreadcrumbsFreeBlock(FfxInterface* backendInterface, FfxBreadcrumbsBlockData* blockData)
	{
		ZE_ASSERT(blockData != nullptr, "Empty FFX Breadcrumbs block data!");
		GetDevice(backendInterface).FreeBreadcrumbsBlock(*blockData);
	}

	void ffxBreadcrumbsWrite(FfxInterface* backendInterface, FfxCommandList commandList, U32 value, U64 gpuLocation, void* gpuBuffer, bool isBegin)
	{
		ZE_ASSERT(gpuBuffer != nullptr, "Empty FFX Breadcrumbs GPU buffer!");
		GetCommandList(commandList).WriteBreadcrumbs(GetDevice(backendInterface), value, gpuLocation, gpuBuffer, isBegin);
	}

	void ffxBreadcrumbsPrintDeviceInfo(FfxInterface* backendInterface, FfxAllocationCallbacks* allocs, bool extendedInfo, char** printBuffer, U64* printSize)
	{
		// TODO: currently no printing of device info
		ZE_FAIL("No device info!");
	}

	FfxErrorCode ffxGetPermutationBlobByIndex(FfxEffect effectId, FfxPass passId, FfxBindStage bindStage, uint32_t permutationOptions, FfxShaderBlob* outBlob)
	{
		if (outBlob)
			return GetShaderInfo(nullptr, effectId, passId, permutationOptions, *outBlob, nullptr);
		return FFX_ERROR_INVALID_POINTER;
	}

	FfxErrorCode ffxSwapChainConfigureFrameGeneration(const FfxFrameGenerationConfig* config)
	{
		ZE_FAIL("FSR frame generation is currently not supported!");
		return FFX_ERROR_INCOMPLETE_INTERFACE;
	}
#pragma endregion
#pragma region FFX utility functions
	constexpr BackendInterface& GetFfxInterface(FfxInterface* backendInterface) noexcept
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(backendInterface->scratchBuffer, "Empty FFX backend interface!");
		return *(BackendInterface*)(backendInterface->scratchBuffer); // No reinterpret_cast to ensure constexpr
	}

	constexpr BackendContext& GetFfxCtx(BackendInterface& ffxInterface) noexcept
	{
		ZE_ASSERT(ffxInterface.Ctx, "Empty FFX backend context!");
		return *ffxInterface.Ctx;
	}

	constexpr BackendContext& GetFfxCtx(FfxInterface* backendInterface) noexcept
	{
		return GetFfxCtx(GetFfxInterface(backendInterface));
	}

	constexpr Device& GetDevice(FfxInterface* backendInterface) noexcept
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(backendInterface->device, "Empty FFX device interface!");
		return *(Device*)(backendInterface->device); // No reinterpret_cast to ensure constexpr
	}

	constexpr CommandList& GetCommandList(FfxCommandList commandList) noexcept
	{
		ZE_ASSERT(commandList, "Empty FFX command list!");
		return *((CommandList*)commandList);
	}

	constexpr Pipeline::FrameResourceType GetResourceType(FfxResourceType type) noexcept
	{
		switch (type)
		{
		default:
		ZE_ENUM_UNHANDLED();
		case FFX_RESOURCE_TYPE_BUFFER:
		return Pipeline::FrameResourceType::Buffer;
		case FFX_RESOURCE_TYPE_TEXTURE1D:
		return Pipeline::FrameResourceType::Texture1D;
		case FFX_RESOURCE_TYPE_TEXTURE2D:
		return Pipeline::FrameResourceType::Texture2D;
		case FFX_RESOURCE_TYPE_TEXTURE_CUBE:
		return Pipeline::FrameResourceType::TextureCube;
		case FFX_RESOURCE_TYPE_TEXTURE3D:
		return Pipeline::FrameResourceType::Texture3D;
		}
	}

	constexpr Pipeline::FrameResourceFlags GetResourceFlags(FfxResourceUsage usage) noexcept
	{
		Pipeline::FrameResourceFlags flags = Base(Pipeline::FrameResourceFlag::ForceSRV);
		if (usage != FFX_RESOURCE_USAGE_READ_ONLY)
		{
			if (usage & (FFX_RESOURCE_USAGE_RENDERTARGET | FFX_RESOURCE_USAGE_DCC_RENDERTARGET))
				flags |= Pipeline::FrameResourceFlag::ForceRTV;
			if (usage & FFX_RESOURCE_USAGE_UAV)
				flags |= Pipeline::FrameResourceFlag::ForceUAV;
			if (usage & FFX_RESOURCE_USAGE_DEPTHTARGET)
				flags |= Pipeline::FrameResourceFlag::ForceDSV;
			if (usage & FFX_RESOURCE_USAGE_INDIRECT)
				flags |= Pipeline::FrameResourceFlag::AllowIndirect;
			if (usage & FFX_RESOURCE_USAGE_ARRAYVIEW)
				flags |= Pipeline::FrameResourceFlag::ArrayView;
			if (usage & FFX_RESOURCE_USAGE_STENCILTARGET)
				flags |= Pipeline::FrameResourceFlag::StencilView | Pipeline::FrameResourceFlag::ForceDSV;
		}
		return flags;
	}

	constexpr Pipeline::TextureLayout GetLayout(FfxResourceStates state) noexcept
	{
		switch (state)
		{
		case FFX_RESOURCE_STATE_COMMON:
		return Pipeline::TextureLayout::Common;
		case FFX_RESOURCE_STATE_UNORDERED_ACCESS:
		return Pipeline::TextureLayout::UnorderedAccess;
		case FFX_RESOURCE_STATE_COMPUTE_READ:
		case FFX_RESOURCE_STATE_PIXEL_READ:
		case FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ:
		return Pipeline::TextureLayout::ShaderResource;
		case FFX_RESOURCE_STATE_COPY_SRC:
		return Pipeline::TextureLayout::CopySource;
		case FFX_RESOURCE_STATE_COPY_DEST:
		return Pipeline::TextureLayout::CopyDest;
		default:
		ZE_FAIL("Unhandled resource state!");
		[[fallthrough]];
		case FFX_RESOURCE_STATE_GENERIC_READ:
		case FFX_RESOURCE_STATE_INDIRECT_ARGUMENT:
		return Pipeline::TextureLayout::GenericRead;
		case FFX_RESOURCE_STATE_PRESENT:
		return Pipeline::TextureLayout::Present;
		case FFX_RESOURCE_STATE_RENDER_TARGET:
		return Pipeline::TextureLayout::RenderTarget;
		}
	}

	constexpr Resource::Texture::AddressMode GetAddressMode(FfxAddressMode mode) noexcept
	{
		switch (mode)
		{
		default:
		ZE_ENUM_UNHANDLED();
		case FFX_ADDRESS_MODE_WRAP:
		return Resource::Texture::AddressMode::Repeat;
		case FFX_ADDRESS_MODE_MIRROR:
		return Resource::Texture::AddressMode::Mirror;
		case FFX_ADDRESS_MODE_CLAMP:
		return Resource::Texture::AddressMode::Edge;
		case FFX_ADDRESS_MODE_BORDER:
		return Resource::Texture::AddressMode::BorderColor;
		case FFX_ADDRESS_MODE_MIRROR_ONCE:
		return Resource::Texture::AddressMode::MirrorOnce;
		}
	}

	constexpr Resource::SamplerFilter GetFilter(FfxFilterType filter) noexcept
	{
		switch (filter)
		{
		default:
		ZE_ENUM_UNHANDLED();
		case FFX_FILTER_TYPE_MINMAGMIP_POINT:
		return Resource::SamplerType::Point;
		case FFX_FILTER_TYPE_MINMAGMIP_LINEAR:
		return Resource::SamplerType::Linear;
		case FFX_FILTER_TYPE_MINMAGLINEARMIP_POINT:
		return Resource::SamplerType::LinearMinification | Resource::SamplerType::LinearMagnification;
		}
	}

	RID GetRID(BackendInterface& ffxInterface, S32 internalIndex) noexcept
	{
		ZE_ASSERT(internalIndex, "Invalid FFX resource index");

		auto* registered = GetFfxCtx(ffxInterface).Resources.try_get<DynamicResource>(GetResID(internalIndex));
		if (registered)
			return registered->ResID;

		ZE_ASSERT(ffxInterface.InternalBuffers.Contains(internalIndex), "Resource has not been created yet!");
		return ffxInterface.InternalBuffers.Get(internalIndex).ResID;
	}

	void AddResourceBarrier(BackendInterface& ffxInterface, S32 internalIndex, FfxResourceStates after) noexcept
	{
		BackendContext& ctx = GetFfxCtx(ffxInterface);
		ResourceStateInfo& state = ctx.Resources.get<ResourceStateInfo>(GetResID(internalIndex));

		if (state.Current != after || state.Undefined)
		{
			constexpr FfxResourceStates COMPUTE_STATES = static_cast<FfxResourceStates>(FFX_RESOURCE_STATE_UNORDERED_ACCESS | FFX_RESOURCE_STATE_COMPUTE_READ);
			constexpr FfxResourceStates GFX_STATES = static_cast<FfxResourceStates>(FFX_RESOURCE_STATE_PIXEL_READ | FFX_RESOURCE_STATE_RENDER_TARGET | FFX_RESOURCE_STATE_DEPTH_ATTACHEMENT);

			// Adjust for resource initialization
			const Pipeline::TextureLayout currentLayout = state.Undefined ? Pipeline::TextureLayout::Undefined : GetLayout(state.Current);
			const Pipeline::TextureLayout afterLayout = GetLayout(after);
			const Pipeline::ResourceAccesses accessBefore = Pipeline::GetAccessFromLayout(currentLayout);
			Pipeline::ResourceAccesses accessAfter = Pipeline::GetAccessFromLayout(afterLayout);
			if (after & FFX_RESOURCE_STATE_INDIRECT_ARGUMENT)
				accessAfter |= Pipeline::ResourceAccess::IndirectArguments;

			ctx.Barriers.emplace_back(GetRID(ffxInterface, internalIndex),
				currentLayout, afterLayout, accessBefore, accessAfter,
				state.Undefined ? Base(Pipeline::StageSync::ComputeShading) : Pipeline::GetSyncFromAccess(accessBefore, state.Current & GFX_STATES, state.Current & COMPUTE_STATES, false),
				Pipeline::GetSyncFromAccess(accessAfter, after & GFX_STATES, after & COMPUTE_STATES, false));
			state.Current = after;
			state.Undefined = false;
		}
		else if (after == FFX_RESOURCE_STATE_UNORDERED_ACCESS)
		{
			constexpr Pipeline::StageSyncs SYNC = Pipeline::GetSyncFromAccess(Base(Pipeline::ResourceAccess::UnorderedAccess), false, false, false);

			ctx.Barriers.emplace_back(GetRID(ffxInterface, internalIndex),
				Pipeline::TextureLayout::UnorderedAccess, Pipeline::TextureLayout::UnorderedAccess,
				Base(Pipeline::ResourceAccess::UnorderedAccess),
				Base(Pipeline::ResourceAccess::UnorderedAccess),
				SYNC, SYNC);
		}
	}

	void FlushBarriers(BackendContext& ctx, CommandList& cl, Pipeline::FrameBuffer& buffers)
	{
		if (ctx.Barriers.size())
		{
			buffers.Barrier(cl, ctx.Barriers.data(), Utils::SafeCast<U32>(ctx.Barriers.size()));
			ctx.Barriers.clear();
		}
	}

	void ExecuteClearJob(BackendInterface& ffxInterface, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxClearFloatJobDescription& job)
	{
		BackendContext& ctx = GetFfxCtx(ffxInterface);
		AddResourceBarrier(ffxInterface, job.target.internalIndex, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		FlushBarriers(ctx, cl, buffers);

		buffers.ClearUAV(cl, GetRID(ffxInterface, job.target.internalIndex), *reinterpret_cast<const ColorF4*>(job.color));
	}

	void ExecuteCopyJob(BackendInterface& ffxInterface, Device& dev, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxCopyJobDescription& job)
	{
		BackendContext& ctx = GetFfxCtx(ffxInterface);
		RID srcId = GetRID(ffxInterface, job.src.internalIndex);
		RID destId = GetRID(ffxInterface, job.dst.internalIndex);
		AddResourceBarrier(ffxInterface, job.src.internalIndex, FFX_RESOURCE_STATE_COPY_SRC);
		AddResourceBarrier(ffxInterface, job.dst.internalIndex, FFX_RESOURCE_STATE_COPY_DEST);
		FlushBarriers(ctx, cl, buffers);

		// Initialization of resource
		if (srcId == destId)
		{
			ResID ffxID = GetResID(job.dst.internalIndex);
			InitData& initData = ctx.Resources.get<InitData>(ffxID);

			if (initData.IsBuffer)
				buffers.InitResource(cl, destId, initData.Buffer);
			else
				buffers.InitResource(cl, destId, initData.Texture, 0);
			initData.LastFrameUsed = Settings::GetFrameIndex();
		}
		else
			buffers.Copy(dev, cl, srcId, destId);
	}

	void ExecuteComputeJob(BackendInterface& ffxInterface, Device& dev, CommandList& cl, Resource::DynamicCBuffer& dynamicBuffer, Pipeline::FrameBuffer& buffers, const FfxComputeJobDescription& job)
	{
		BackendContext& ctx = GetFfxCtx(ffxInterface);
		// Transition all the UAVs and SRVs
		for (U32 i = 0; i < job.pipeline.uavBufferCount; ++i)
			if (job.uavBuffers[i].resource.internalIndex)
				AddResourceBarrier(ffxInterface, job.uavBuffers[i].resource.internalIndex, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		for (U32 i = 0; i < job.pipeline.uavTextureCount; ++i)
			if (job.uavTextures[i].resource.internalIndex)
				AddResourceBarrier(ffxInterface, job.uavTextures[i].resource.internalIndex, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		for (U32 i = 0; i < job.pipeline.srvBufferCount; ++i)
			if (job.srvBuffers[i].resource.internalIndex)
				AddResourceBarrier(ffxInterface, job.srvBuffers[i].resource.internalIndex, FFX_RESOURCE_STATE_COMPUTE_READ);
		for (U32 i = 0; i < job.pipeline.srvTextureCount; ++i)
			if (job.srvTextures[i].resource.internalIndex)
				AddResourceBarrier(ffxInterface, job.srvTextures[i].resource.internalIndex, FFX_RESOURCE_STATE_COMPUTE_READ);

		// If we are dispatching indirectly, transition the argument resource to indirect argument
		if (job.pipeline.cmdSignature)
			AddResourceBarrier(ffxInterface, job.cmdArgument.internalIndex, FFX_RESOURCE_STATE_INDIRECT_ARGUMENT);
		FlushBarriers(ctx, cl, buffers);

		// Bind pipeline with binding schema
		ctx.Pipelines.Get(reinterpret_cast<U64>(job.pipeline.pipeline)).Bind(cl);
		Binding::Context bindCtx = { ctx.Bindings.Get(reinterpret_cast<U64>(job.pipeline.rootSignature)) };
		bindCtx.BindingSchema.SetCompute(cl);

		// Bind all resources
		for (U32 i = 0; i < job.pipeline.uavBufferCount; ++i)
		{
			if (job.uavBuffers[i].resource.internalIndex)
				buffers.SetUAV(cl, bindCtx, GetRID(ffxInterface, job.uavBuffers[i].resource.internalIndex));
			else
				++bindCtx.Count;
		}
		for (U32 i = 0; i < job.pipeline.uavTextureCount; ++i)
		{
			if (job.uavTextures[i].resource.internalIndex)
			{
				RID rid = GetRID(ffxInterface, job.uavTextures[i].resource.internalIndex);
				if (job.uavTextures[i].mip)
					buffers.SetUAV(cl, bindCtx, rid, Utils::SafeCast<U16>(job.uavTextures[i].mip));
				else
					buffers.SetUAV(cl, bindCtx, rid);
			}
			else
				++bindCtx.Count;
		}
		for (U32 i = 0; i < job.pipeline.srvBufferCount; ++i)
		{
			if (job.srvBuffers[i].resource.internalIndex)
				buffers.SetSRV(cl, bindCtx, GetRID(ffxInterface, job.srvBuffers[i].resource.internalIndex));
			else
				++bindCtx.Count;
		}
		for (U32 i = 0; i < job.pipeline.srvTextureCount; ++i)
		{
			if (job.srvTextures[i].resource.internalIndex)
				buffers.SetSRV(cl, bindCtx, GetRID(ffxInterface, job.srvTextures[i].resource.internalIndex));
			else
				++bindCtx.Count;
		}

		// Bind previously copied data
		for (U32 i = 0; i < job.pipeline.constCount; ++i)
			dynamicBuffer.Bind(cl, bindCtx, { job.cbs[i].num32BitEntries, reinterpret_cast<U64>(job.cbs[i].data) });

		// Dispatch (or dispatch indirect)
		if (job.pipeline.cmdSignature)
		{
			buffers.ExecuteIndirect(cl,
				ctx.CommandSignatures.Get(static_cast<IndirectCommandType>(reinterpret_cast<U64>(job.pipeline.cmdSignature))),
				GetRID(ffxInterface, job.cmdArgument.internalIndex), job.cmdArgumentOffset);
		}
		else
			cl.Compute(dev, job.dimensions[0], job.dimensions[1], job.dimensions[2]);
	}

	void ExecuteBarrierJob(BackendInterface& ffxInterface, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxBarrierDescription& job)
	{
		if (job.barrierType == FFX_BARRIER_TYPE_UAV)
		{
			constexpr Pipeline::StageSyncs SYNC = Pipeline::GetSyncFromAccess(Base(Pipeline::ResourceAccess::UnorderedAccess), false, false, false);

			buffers.Barrier(cl, { GetRID(ffxInterface, job.resource.internalIndex),
				Pipeline::TextureLayout::UnorderedAccess, Pipeline::TextureLayout::UnorderedAccess,
				Base(Pipeline::ResourceAccess::UnorderedAccess),
				Base(Pipeline::ResourceAccess::UnorderedAccess),
				SYNC, SYNC, Pipeline::BarrierType::Immediate, job.subResourceID });
		}
		else if (job.currentState != job.newState)
		{
			const Pipeline::TextureLayout currentLayout = GetLayout(job.currentState);
			const Pipeline::TextureLayout afterLayout = GetLayout(job.newState);
			const Pipeline::ResourceAccesses accessBefore = Pipeline::GetAccessFromLayout(currentLayout);
			Pipeline::ResourceAccesses accessAfter = Pipeline::GetAccessFromLayout(afterLayout);
			if (job.newState & FFX_RESOURCE_STATE_INDIRECT_ARGUMENT)
				accessAfter |= Pipeline::ResourceAccess::IndirectArguments;

			buffers.Barrier(cl, { GetRID(ffxInterface, job.resource.internalIndex),
				currentLayout, afterLayout, accessBefore, accessAfter,
				Pipeline::GetSyncFromAccess(accessBefore, job.currentState & FFX_RESOURCE_STATE_PIXEL_READ, job.currentState & FFX_RESOURCE_STATE_COMPUTE_READ, false),
				Pipeline::GetSyncFromAccess(accessAfter, job.newState & FFX_RESOURCE_STATE_PIXEL_READ, job.newState & FFX_RESOURCE_STATE_COMPUTE_READ, false),
				Pipeline::BarrierType::Immediate, job.subResourceID });

			ResourceStateInfo& state = GetFfxCtx(ffxInterface).Resources.get<ResourceStateInfo>(GetResID(job.resource.internalIndex));
			state.Current = job.newState;
			state.Undefined = false;
		}
	}

	void ExecuteDiscardJob(BackendInterface& ffxInterface, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxDiscardJobDescription& job)
	{
		ResourceStateInfo& state = GetFfxCtx(ffxInterface).Resources.get<ResourceStateInfo>(GetResID(job.target.internalIndex));
		state.Undefined = false;
		const Pipeline::TextureLayout currentLayout = GetLayout(state.Current);
		const Pipeline::ResourceAccesses accessBefore = Pipeline::GetAccessFromLayout(currentLayout);

		buffers.Barrier(cl, { GetRID(ffxInterface, job.target.internalIndex),
			Pipeline::TextureLayout::Undefined, currentLayout,
			Base(Pipeline::ResourceAccess::None), accessBefore,
			Base(Pipeline::StageSync::None),
			Pipeline::GetSyncFromAccess(accessBefore, state.Current & FFX_RESOURCE_STATE_PIXEL_READ, state.Current & FFX_RESOURCE_STATE_COMPUTE_READ, false),
			Pipeline::BarrierType::Immediate, UINT32_MAX });
	}
#pragma endregion
}