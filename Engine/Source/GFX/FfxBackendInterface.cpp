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
	struct FfxResourceName
	{
		wchar_t Name[FFX_RESOURCE_NAME_SIZE];
	};
	// Data about current and starting resource state
	struct FfxResourceStateInfo
	{
		FfxResourceStates Current;
		FfxResourceStates Initial;
		bool Undefined;
	};
	// Initial data for filling up GPU resources
	struct FfxInitData
	{
		bool IsBuffer;
		Resource::CBuffer Buffer;
		Resource::Texture::Pack Texture;
		U64 LastFrameUsed;
	};
	// Tag for resources registered per frame from outside
	struct FfxDynamicResource
	{
		RID ResID;
	};
	// ID of internally created resource
	typedef U32 FfxResID;

	// Main context used by FFX SDK
	struct FfxBackendContext
	{
		entt::basic_registry<FfxResID> Resources;
		Data::Library<U64, Resource::PipelineStateCompute> Pipelines;
		Data::Library<U64, U64> PipelinesReferences;
		Data::Library<U64, Binding::Schema> Bindings;
		Data::Library<U64, U64> BindingsReferences;
		Data::Library<IndirectCommandType, CommandSignature> CommandSignatures;
		Data::Library<IndirectCommandType, U64> CommandSignaturesReferences;
		std::vector<Pipeline::BarrierTransition> Barriers;
		std::vector<FfxGpuJobDescription> Jobs;
	};

	// Interface data setup when filling FfxInterface
	struct FfxBackendInterface
	{
		ChainPool<Resource::DynamicCBuffer>& DynamicBuffers;
		Pipeline::FrameBuffer& Buffers;
		IO::DiskManager& Disk;
		Data::Library<S32, FFX::InternalResourceDescription>& InternalBuffers;
		bool& NotifyBuffersChange;
		U32 ContextRefCount = 0;
		PassInfo CurrentPass = {};
		FfxBackendContext* Ctx = nullptr;
	};

	// Interface functions used by FFX SDK backend
	void ffxAssertCallback(const char* message) noexcept;
	FfxUInt32 ffxGetSDKVersion(FfxInterface* backendInterface);
	FfxErrorCode ffxCreateBackendContext(FfxInterface* backendInterface, FfxUInt32* effectContextId);
	FfxErrorCode ffxGetDeviceCapabilities(FfxInterface* backendInterface, FfxDeviceCapabilities* deviceCapabilities);
	FfxErrorCode ffxDestroyBackendContext(FfxInterface* backendInterface, FfxUInt32 effectContextId);
	FfxErrorCode ffxCreateResource(FfxInterface* backendInterface,
		const FfxCreateResourceDescription* createResourceDescription,
		FfxUInt32 effectContextId, FfxResourceInternal* outTexture);
	FfxErrorCode ffxDestroyResource(FfxInterface* backendInterface, FfxResourceInternal resource);
	FfxResource ffxGetResource(FfxInterface* backendInterface, FfxResourceInternal resource);
	FfxErrorCode ffxRegisterResource(FfxInterface* backendInterface, const FfxResource* inResource,
		FfxUInt32 effectContextId, FfxResourceInternal* outResourceInternal);
	FfxErrorCode ffxUnregisterResources(FfxInterface* backendInterface, FfxCommandList commandList, FfxUInt32 effectContextId);
	FfxResourceDescription ffxGetResourceDescriptor(FfxInterface* backendInterface, FfxResourceInternal resource);
	FfxErrorCode ffxCreatePipeline(FfxInterface* backendInterface, FfxEffect effect, FfxPass passId,
		uint32_t permutationOptions, const FfxPipelineDescription* desc, FfxUInt32 effectContextId, FfxPipelineState* outPipeline);
	FfxErrorCode ffxDestroyPipeline(FfxInterface* backendInterface, FfxPipelineState* pipeline, FfxUInt32 effectContextId);
	FfxErrorCode ffxScheduleGpuJob(FfxInterface* backendInterface, const FfxGpuJobDescription* job);
	FfxErrorCode ffxExecuteGpuJobs(FfxInterface* backendInterface, FfxCommandList commandList);

	// Utility functions for working with FFX SDK
	constexpr FfxResID GetFfxResID(S32 internalIndex) noexcept { ZE_ASSERT(internalIndex, "Invalid FFX resource index"); return static_cast<FfxResID>(internalIndex - 1); }
	constexpr S32 GetInternalIndex(FfxResID ffxID) noexcept { return static_cast<S32>(ffxID + 1); }
	// Fix RID for special values to treat it like a pointer
	constexpr void* EncodeRID(RID rid) noexcept { return (void*)static_cast<uintptr_t>(rid == INVALID_RID ? 0 : (rid == BACKBUFFER_RID ? INVALID_RID : rid)); } // No reinterpret_cast to ensure constexpr
	constexpr RID DecodeRID(void* resource) noexcept { return resource ? (((RID)(uintptr_t)(resource)) == INVALID_RID ? BACKBUFFER_RID : (RID)((uintptr_t)resource)) : INVALID_RID; } // No reinterpret_cast to ensure constexpr

	constexpr FfxBackendInterface& GetFfxInterface(FfxInterface* backendInterface) noexcept;
	constexpr FfxBackendContext& GetFfxCtx(FfxBackendInterface& ffxInterface) noexcept;
	constexpr FfxBackendContext& GetFfxCtx(FfxInterface* backendInterface) noexcept;
	constexpr Device& GetDevice(FfxInterface* backendInterface) noexcept;
	constexpr CommandList& GetCommandList(FfxCommandList commandList) noexcept;
	constexpr Pipeline::FrameResourceType GetResourceType(FfxResourceType type) noexcept;
	constexpr Pipeline::FrameResourceFlags GetResourceFlags(FfxResourceUsage usage) noexcept;
	constexpr Pipeline::TextureLayout GetLayout(FfxResourceStates state) noexcept;
	constexpr Resource::Texture::AddressMode GetAddressMode(FfxAddressMode mode) noexcept;
	constexpr Resource::SamplerFilter GetFilter(FfxFilterType filter) noexcept;
	RID GetRID(FfxBackendInterface& ffxInterface, S32 internalIndex) noexcept;
	void AddResourceBarrier(FfxBackendInterface& ffxInterface, S32 internalIndex, FfxResourceStates after) noexcept;
	void FlushBarriers(FfxBackendContext& ctx, CommandList& cl, Pipeline::FrameBuffer& buffers);
	void ExecuteClearJob(FfxBackendInterface& ffxInterface, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxClearFloatJobDescription& job);
	void ExecuteCopyJob(FfxBackendInterface& ffxInterface, Device& dev, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxCopyJobDescription& job);
	void ExecuteComputeJob(FfxBackendInterface& ffxInterface, Device& dev, CommandList& cl, Resource::DynamicCBuffer& dynamicBuffer, Pipeline::FrameBuffer& buffers, const FfxComputeJobDescription& job);

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
		backendInterface.fpCreateBackendContext = ffxCreateBackendContext;
		backendInterface.fpGetDeviceCapabilities = ffxGetDeviceCapabilities;
		backendInterface.fpDestroyBackendContext = ffxDestroyBackendContext;
		backendInterface.fpCreateResource = ffxCreateResource;
		backendInterface.fpDestroyResource = ffxDestroyResource;
		backendInterface.fpGetResource = ffxGetResource;
		backendInterface.fpRegisterResource = ffxRegisterResource;
		backendInterface.fpUnregisterResources = ffxUnregisterResources;
		backendInterface.fpGetResourceDescription = ffxGetResourceDescriptor;
		backendInterface.fpCreatePipeline = ffxCreatePipeline;
		backendInterface.fpDestroyPipeline = ffxDestroyPipeline;
		backendInterface.fpScheduleGpuJob = ffxScheduleGpuJob;
		backendInterface.fpExecuteGpuJobs = ffxExecuteGpuJobs;

		// Setup all custom backend memory
		backendInterface.scratchBufferSize = sizeof(FfxBackendInterface);
		backendInterface.scratchBuffer = new FfxBackendInterface{ dynamicBuffers, frameBuffer, disk, internalBuffers, notifyBuffersChange };
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
			delete reinterpret_cast<FfxBackendInterface*>(backendInterface.scratchBuffer);
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

	FfxErrorCode ffxCreateBackendContext(FfxInterface* backendInterface, FfxUInt32* effectContextId)
	{
		FfxBackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		if (effectContextId)
			*effectContextId = ffxInterface.CurrentPass.PassID;

		if (ffxInterface.ContextRefCount++ == 0)
		{
			U8* buffer = new U8[sizeof(FfxBackendContext)];
			std::memset(buffer, 0, sizeof(FfxBackendContext));
			reinterpret_cast<FfxBackendInterface*>(backendInterface->scratchBuffer)->Ctx = reinterpret_cast<FfxBackendContext*>(buffer);

			new(reinterpret_cast<FfxBackendContext*>(buffer)) FfxBackendContext;
		}

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
		case ShaderModel::V6_9:
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

	FfxErrorCode ffxDestroyBackendContext(FfxInterface* backendInterface, FfxUInt32 effectContextId)
	{
		FfxBackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		if (--ffxInterface.ContextRefCount == 0)
		{
			Device& dev = GetDevice(backendInterface);
			FfxBackendContext& ctx = GetFfxCtx(backendInterface);

			if (ffxInterface.InternalBuffers.Size())
			{
				ffxInterface.InternalBuffers.Clear();
				ffxInterface.NotifyBuffersChange = true;
			}

			// Free all remaining init resources
			for (FfxResID ffxId : ctx.Resources.view<FfxInitData>())
			{
				FfxInitData& initData = ctx.Resources.get<FfxInitData>(ffxId);
				initData.IsBuffer ? initData.Buffer.Free(dev) : initData.Texture.Free(dev);
			}
			ctx.Resources.clear();

			ctx.Pipelines.Transform([&dev](Resource::PipelineStateCompute& pipeline) { pipeline.Free(dev); });
			ctx.Bindings.Transform([&dev](Binding::Schema& schema) { schema.Free(dev); });
			ctx.CommandSignatures.Transform([&dev](CommandSignature& signature) { signature.Free(dev); });

			ctx.~FfxBackendContext();
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

		FfxBackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		FfxBackendContext& ctx = GetFfxCtx(ffxInterface);
		Device& dev = GetDevice(backendInterface);

		FfxResID ffxID = ctx.Resources.create();
		outTexture->internalIndex = GetInternalIndex(ffxID);

		ZE_ASSERT(createResourceDescription->heapType == FFX_HEAP_TYPE_DEFAULT, "Need to account for new type of heap!");
		ZE_ASSERT(!ffxInterface.InternalBuffers.Contains(outTexture->internalIndex), "Resource has been already created!");

		Pipeline::FrameResourceDesc resDesc = {};
		if (createResourceDescription->resourceDescription.type == FFX_RESOURCE_TYPE_BUFFER)
		{
			resDesc.Sizes.X = createResourceDescription->resourceDescription.size;
			resDesc.Sizes.Y = createResourceDescription->resourceDescription.stride;
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

		resDesc.Format = GetPixelFormat(createResourceDescription->resourceDescription.format);
		resDesc.ClearColor = ColorF4{};
		resDesc.ClearDepth = 0.0f;
		resDesc.ClearStencil = 0;
		resDesc.MipLevels = Utils::SafeCast<U16>(createResourceDescription->resourceDescription.mipCount);
		resDesc.Type = GetResourceType(createResourceDescription->resourceDescription.type);
		ZE_FRAME_RES_SET_NAME(resDesc, Utils::ToUTF8(createResourceDescription->name));

		if (createResourceDescription->initData)
		{
			FfxInitData& initData = ctx.Resources.emplace<FfxInitData>(ffxID,
				createResourceDescription->resourceDescription.type == FFX_RESOURCE_TYPE_BUFFER);
			initData.LastFrameUsed = UINT64_MAX; // Indication that it shouldn't be removed
			if (initData.IsBuffer)
			{
				Resource::CBufferData data = {};
				data.Bytes = createResourceDescription->initDataSize;
				data.DataStatic = createResourceDescription->initData;
				initData.Buffer.Init(dev, ffxInterface.Disk, data);
			}
			else
			{
				// Handling of textures
				std::vector<Surface> surfaces;
				surfaces.emplace_back(resDesc.Sizes.X, resDesc.Sizes.Y, resDesc.DepthOrArraySize, resDesc.MipLevels,
					static_cast<U16>(1U), resDesc.Format, false, createResourceDescription->initData);
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

		ctx.Resources.emplace<FfxResourceStateInfo>(ffxID, createResourceDescription->initalState, createResourceDescription->initalState, true);
		ctx.Resources.emplace<FfxResourceDescription>(ffxID, createResourceDescription->resourceDescription);
#if _ZE_DEBUG_GFX_NAMES
		if (createResourceDescription->name)
			wcscpy_s(ctx.Resources.emplace<FfxResourceName>(ffxID).Name, createResourceDescription->name);
#endif
		return FFX_OK;
	}

	FfxErrorCode ffxDestroyResource(FfxInterface* backendInterface, FfxResourceInternal resource)
	{
		if (resource.internalIndex)
		{
			FfxBackendInterface& ffxInterface = GetFfxInterface(backendInterface);
			FfxBackendContext& ctx = GetFfxCtx(ffxInterface);
			FfxResID id = GetFfxResID(resource.internalIndex);

			// Due to different handling of internal resources sometimes there can be request to delete resource that is not present (eg. FSR2 copy resources)
			if (ctx.Resources.valid(id))
			{
				Device& dev = GetDevice(backendInterface);
				ZE_ASSERT(ffxInterface.InternalBuffers.Contains(resource.internalIndex), "Resource has not been properly created!");

				FfxInitData* initData = ctx.Resources.try_get<FfxInitData>(id);
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
		FfxBackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		FfxBackendContext& ctx = GetFfxCtx(ffxInterface);
		FfxResID ffxId = GetFfxResID(resource.internalIndex);

		FfxResource res = {};
		res.resource = EncodeRID(GetRID(ffxInterface, resource.internalIndex));
		res.state = ctx.Resources.get<FfxResourceStateInfo>(ffxId).Current;
		res.description = ffxGetResourceDescriptor(backendInterface, resource);
#if _ZE_DEBUG_GFX_NAMES
		if (FfxResourceName* name = ctx.Resources.try_get<FfxResourceName>(ffxId))
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
			FfxBackendContext& ctx = GetFfxCtx(backendInterface);
			FfxResID ffxId = ctx.Resources.create();
			outResourceInternal->internalIndex = GetInternalIndex(ffxId);

			ctx.Resources.emplace<FfxResourceStateInfo>(ffxId, inResource->state, inResource->state, false);
			ctx.Resources.emplace<FfxResourceDescription>(ffxId, inResource->description);
#if _ZE_DEBUG_GFX_NAMES
			if (inResource->name)
				wcscpy_s(ctx.Resources.emplace<FfxResourceName>(ffxId).Name, inResource->name);
#endif
			// Tag as dynamic per-frame resource
			ctx.Resources.emplace<FfxDynamicResource>(ffxId, DecodeRID(inResource->resource));
		}
		else
			outResourceInternal->internalIndex = 0;
		return FFX_OK;
	}

	FfxErrorCode ffxUnregisterResources(FfxInterface* backendInterface, FfxCommandList commandList, FfxUInt32 effectContextId)
	{
		FfxBackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		FfxBackendContext& ctx = GetFfxCtx(ffxInterface);

		// Walk back all the resources that don't belong to FFX and reset them to their initial state
		for (FfxResID res : ctx.Resources.view<FfxDynamicResource>())
			AddResourceBarrier(ffxInterface, GetInternalIndex(res), ctx.Resources.get<FfxResourceStateInfo>(res).Initial);
		FlushBarriers(ctx, GetCommandList(commandList), GetFfxInterface(backendInterface).Buffers);

		// Clear dynamic resources
		ctx.Resources.destroy(ctx.Resources.view<FfxDynamicResource>().begin(), ctx.Resources.view<FfxDynamicResource>().end());
		return FFX_OK;
	}

	FfxResourceDescription ffxGetResourceDescriptor(FfxInterface* backendInterface, FfxResourceInternal resource)
	{
		return GetFfxCtx(backendInterface).Resources.get<FfxResourceDescription>(GetFfxResID(resource.internalIndex));
	}

	FfxErrorCode ffxCreatePipeline(FfxInterface* backendInterface, FfxEffect effect, FfxPass passId,
		uint32_t permutationOptions, const FfxPipelineDescription* desc, FfxUInt32 effectContextId, FfxPipelineState* outPipeline)
	{
		ZE_ASSERT(desc, "Empty FFX pipeline desc!");
		ZE_ASSERT(outPipeline, "Empty FFX pipeline state!");
		ZE_ASSERT(desc->stage == FFX_BIND_COMPUTE_SHADER_STAGE || desc->stage == 0, "Pipeline not for the compute shader!");

		Device& dev = GetDevice(backendInterface);
		FfxBackendContext& ctx = GetFfxCtx(backendInterface);
		const U64 id = FFX::GetPipelineID(effect, passId, permutationOptions);

		FfxShaderBlob shaderBlob = {};
		if (!ctx.Pipelines.Contains(id))
		{
			// Fill out for every shader that is created when needed by effect
			Resource::Shader shader;
			const FfxErrorCode code = FFX::GetShaderInfo(dev, effect, passId, permutationOptions, shaderBlob, &shader);
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
			ctx.Bindings.Add(id, dev, schemaDesc);
			ctx.BindingsReferences.Add(id, 0ULL);

			// Create pipeline
			ctx.Pipelines.Add(id, dev, shader, ctx.Bindings.Get(id));
			ctx.PipelinesReferences.Add(id, 0ULL);
			shader.Free(dev);
		}
		else
		{
			// Shader already loaded but resource info still needed
			const FfxErrorCode code = FFX::GetShaderInfo(dev, effect, passId, permutationOptions, shaderBlob, nullptr);
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
			outPipeline->uavTextureBindings[i].bindCount = shaderBlob.boundUAVTextureCounts[i];
			wcscpy_s(outPipeline->uavTextureBindings[i].name, Utils::ToUTF16(shaderBlob.boundUAVTextureNames[i]).c_str());
		}
		for (U32 i = 0; i < shaderBlob.srvTextureCount; ++i)
		{
			outPipeline->srvTextureBindings[i].slotIndex = shaderBlob.boundSRVTextures[i];
			outPipeline->srvTextureBindings[i].bindCount = shaderBlob.boundSRVTextureCounts[i];
			wcscpy_s(outPipeline->srvTextureBindings[i].name, Utils::ToUTF16(shaderBlob.boundSRVTextureNames[i]).c_str());
		}
		for (U32 i = 0; i < shaderBlob.srvBufferCount; ++i)
		{
			outPipeline->srvBufferBindings[i].slotIndex = shaderBlob.boundSRVBuffers[i];
			outPipeline->srvBufferBindings[i].bindCount = shaderBlob.boundSRVBufferCounts[i];
			wcscpy_s(outPipeline->srvBufferBindings[i].name, Utils::ToUTF16(shaderBlob.boundSRVBufferNames[i]).c_str());
		}
		for (U32 i = 0; i < shaderBlob.uavBufferCount; ++i)
		{
			outPipeline->uavBufferBindings[i].slotIndex = shaderBlob.boundUAVBuffers[i];
			outPipeline->uavBufferBindings[i].bindCount = shaderBlob.boundUAVBufferCounts[i];
			wcscpy_s(outPipeline->uavBufferBindings[i].name, Utils::ToUTF16(shaderBlob.boundUAVBufferNames[i]).c_str());
		}
		for (U32 i = 0; i < shaderBlob.cbvCount; ++i)
		{
			outPipeline->constantBufferBindings[i].slotIndex = shaderBlob.boundConstantBuffers[i];
			outPipeline->constantBufferBindings[i].bindCount = shaderBlob.boundConstantBufferCounts[i];
			wcscpy_s(outPipeline->constantBufferBindings[i].name, Utils::ToUTF16(shaderBlob.boundConstantBufferNames[i]).c_str());
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
			FfxBackendContext& ctx = GetFfxCtx(backendInterface);

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

		FfxBackendContext& ctx = GetFfxCtx(backendInterface);

		FfxGpuJobDescription& newJob = ctx.Jobs.emplace_back(*job);
		if (newJob.jobType == FFX_GPU_JOB_COMPUTE)
		{
			// Need to copy CBV data in case it's on the stack only
			for (U32 i = 0; i < newJob.computeJobDescriptor.pipeline.constCount; ++i)
			{
				const U32 entryCount = newJob.computeJobDescriptor.cbs[i].num32BitEntries = job->computeJobDescriptor.cbs[i].num32BitEntries;
				std::memcpy(newJob.computeJobDescriptor.cbs[i].data, job->computeJobDescriptor.cbs[i].data, entryCount * sizeof(U32));
			}
		}
		return FFX_OK;
	}

	FfxErrorCode ffxExecuteGpuJobs(FfxInterface* backendInterface, FfxCommandList commandList)
	{
		FfxBackendInterface& ffxInterface = GetFfxInterface(backendInterface);
		FfxBackendContext& ctx = GetFfxCtx(ffxInterface);
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
			default:
				ZE_FAIL("Unknown FFX GPU job!");
				break;
			}
		}
		ctx.Jobs.clear();

		for (FfxResID ffxId : ctx.Resources.view<FfxInitData>())
		{
			FfxInitData& initData = ctx.Resources.get<FfxInitData>(ffxId);
			if (initData.LastFrameUsed + Settings::GetBackbufferCount() < Settings::GetFrameIndex())
			{
				initData.IsBuffer ? initData.Buffer.Free(dev) : initData.Texture.Free(dev);
				ctx.Resources.remove<FfxInitData>(ffxId);
			}
		}

		return FFX_OK;
	}
#pragma endregion
#pragma region FFX utility functions
	constexpr FfxBackendInterface& GetFfxInterface(FfxInterface* backendInterface) noexcept
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(backendInterface->scratchBuffer, "Empty FFX backend interface!");
		return *(FfxBackendInterface*)(backendInterface->scratchBuffer); // No reinterpret_cast to ensure constexpr
	}

	constexpr FfxBackendContext& GetFfxCtx(FfxBackendInterface& ffxInterface) noexcept
	{
		ZE_ASSERT(ffxInterface.Ctx, "Empty FFX backend context!");
		return *ffxInterface.Ctx;
	}

	constexpr FfxBackendContext& GetFfxCtx(FfxInterface* backendInterface) noexcept
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
			if (usage & FFX_RESOURCE_USAGE_RENDERTARGET)
				flags |= Pipeline::FrameResourceFlag::ForceRTV;
			if (usage & FFX_RESOURCE_USAGE_UAV)
				flags |= Pipeline::FrameResourceFlag::ForceUAV;
			if (usage & FFX_RESOURCE_USAGE_DEPTHTARGET)
				flags |= Pipeline::FrameResourceFlag::ForceDSV;
			if (usage & FFX_RESOURCE_USAGE_INDIRECT)
				flags |= Pipeline::FrameResourceFlag::AllowIndirect;
			if (usage & FFX_RESOURCE_USAGE_ARRAYVIEW)
				flags |= Pipeline::FrameResourceFlag::ArrayView;
		}
		return flags;
	}

	constexpr Pipeline::TextureLayout GetLayout(FfxResourceStates state) noexcept
	{
		switch (state)
		{
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

	RID GetRID(FfxBackendInterface& ffxInterface, S32 internalIndex) noexcept
	{
		ZE_ASSERT(internalIndex, "Invalid FFX resource index");

		auto* registered = GetFfxCtx(ffxInterface).Resources.try_get<FfxDynamicResource>(GetFfxResID(internalIndex));
		if (registered)
			return registered->ResID;

		ZE_ASSERT(ffxInterface.InternalBuffers.Contains(internalIndex), "Resource has not been created yet!");
		return ffxInterface.InternalBuffers.Get(internalIndex).ResID;
	}

	void AddResourceBarrier(FfxBackendInterface& ffxInterface, S32 internalIndex, FfxResourceStates after) noexcept
	{
		FfxBackendContext& ctx = GetFfxCtx(ffxInterface);
		FfxResourceStateInfo& state = ctx.Resources.get<FfxResourceStateInfo>(GetFfxResID(internalIndex));

		if (state.Current != after || state.Undefined)
		{
			// Adjust for resource initialization
			const Pipeline::TextureLayout currentLayout = state.Undefined ? Pipeline::TextureLayout::Undefined : GetLayout(state.Current);
			const Pipeline::TextureLayout afterLayout = GetLayout(after);
			const Pipeline::ResourceAccesses accessBefore = Pipeline::GetAccessFromLayout(currentLayout);
			Pipeline::ResourceAccesses accessAfter = Pipeline::GetAccessFromLayout(afterLayout);
			if (after & FFX_RESOURCE_STATE_INDIRECT_ARGUMENT)
				accessAfter |= Pipeline::ResourceAccess::IndirectArguments;

			ctx.Barriers.emplace_back(GetRID(ffxInterface, internalIndex),
				currentLayout, afterLayout, accessBefore, accessAfter,
				Pipeline::GetSyncFromAccess(accessBefore, state.Current & FFX_RESOURCE_STATE_PIXEL_READ, state.Current & FFX_RESOURCE_STATE_COMPUTE_READ, false),
				Pipeline::GetSyncFromAccess(accessAfter, after & FFX_RESOURCE_STATE_PIXEL_READ, after & FFX_RESOURCE_STATE_COMPUTE_READ, false));
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

	void FlushBarriers(FfxBackendContext& ctx, CommandList& cl, Pipeline::FrameBuffer& buffers)
	{
		if (ctx.Barriers.size())
		{
			buffers.Barrier(cl, ctx.Barriers.data(), Utils::SafeCast<U32>(ctx.Barriers.size()));
			ctx.Barriers.clear();
		}
	}

	void ExecuteClearJob(FfxBackendInterface& ffxInterface, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxClearFloatJobDescription& job)
	{
		FfxBackendContext& ctx = GetFfxCtx(ffxInterface);
		AddResourceBarrier(ffxInterface, job.target.internalIndex, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		FlushBarriers(ctx, cl, buffers);

		buffers.ClearUAV(cl, GetRID(ffxInterface, job.target.internalIndex), *reinterpret_cast<const ColorF4*>(job.color));
	}

	void ExecuteCopyJob(FfxBackendInterface& ffxInterface, Device& dev, CommandList& cl, Pipeline::FrameBuffer& buffers, const FfxCopyJobDescription& job)
	{
		FfxBackendContext& ctx = GetFfxCtx(ffxInterface);
		RID srcId = GetRID(ffxInterface, job.src.internalIndex);
		RID destId = GetRID(ffxInterface, job.dst.internalIndex);
		AddResourceBarrier(ffxInterface, job.src.internalIndex, FFX_RESOURCE_STATE_COPY_SRC);
		AddResourceBarrier(ffxInterface, job.dst.internalIndex, FFX_RESOURCE_STATE_COPY_DEST);
		FlushBarriers(ctx, cl, buffers);

		// Initialization of resource
		if (srcId == destId)
		{
			FfxResID ffxID = GetFfxResID(job.dst.internalIndex);
			FfxInitData& initData = ctx.Resources.get<FfxInitData>(ffxID);

			if (initData.IsBuffer)
				buffers.InitResource(cl, destId, initData.Buffer);
			else
				buffers.InitResource(cl, destId, initData.Texture, 0);
			initData.LastFrameUsed = Settings::GetFrameIndex();
		}
		else
			buffers.Copy(dev, cl, srcId, destId);
	}

	void ExecuteComputeJob(FfxBackendInterface& ffxInterface, Device& dev, CommandList& cl, Resource::DynamicCBuffer& dynamicBuffer, Pipeline::FrameBuffer& buffers, const FfxComputeJobDescription& job)
	{
		FfxBackendContext& ctx = GetFfxCtx(ffxInterface);
		// Transition all the UAVs and SRVs
		for (U32 i = 0; i < job.pipeline.uavBufferCount; ++i)
			if (job.uavBuffers[i].internalIndex)
				AddResourceBarrier(ffxInterface, job.uavBuffers[i].internalIndex, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		for (U32 i = 0; i < job.pipeline.uavTextureCount; ++i)
			if (job.uavTextures[i].internalIndex)
				AddResourceBarrier(ffxInterface, job.uavTextures[i].internalIndex, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		for (U32 i = 0; i < job.pipeline.srvBufferCount; ++i)
			if (job.srvBuffers[i].internalIndex)
				AddResourceBarrier(ffxInterface, job.srvBuffers[i].internalIndex, FFX_RESOURCE_STATE_COMPUTE_READ);
		for (U32 i = 0; i < job.pipeline.srvTextureCount; ++i)
			if (job.srvTextures[i].internalIndex)
				AddResourceBarrier(ffxInterface, job.srvTextures[i].internalIndex, FFX_RESOURCE_STATE_COMPUTE_READ);

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
			if (job.uavBuffers[i].internalIndex)
				buffers.SetUAV(cl, bindCtx, GetRID(ffxInterface, job.uavBuffers[i].internalIndex));
			else
				++bindCtx.Count;
		}
		for (U32 i = 0; i < job.pipeline.uavTextureCount; ++i)
		{
			if (job.uavTextures[i].internalIndex)
			{
				RID rid = GetRID(ffxInterface, job.uavTextures[i].internalIndex);
				if (job.uavTextureMips[i])
					buffers.SetUAV(cl, bindCtx, rid, Utils::SafeCast<U16>(job.uavTextureMips[i]));
				else
					buffers.SetUAV(cl, bindCtx, rid);
			}
			else
				++bindCtx.Count;
		}
		for (U32 i = 0; i < job.pipeline.srvBufferCount; ++i)
		{
			if (job.srvBuffers[i].internalIndex)
				buffers.SetSRV(cl, bindCtx, GetRID(ffxInterface, job.srvBuffers[i].internalIndex));
			else
				++bindCtx.Count;
		}
		for (U32 i = 0; i < job.pipeline.srvTextureCount; ++i)
		{
			if (job.srvTextures[i].internalIndex)
				buffers.SetSRV(cl, bindCtx, GetRID(ffxInterface, job.srvTextures[i].internalIndex));
			else
				++bindCtx.Count;
		}

		// Copy data to dynamic cbuffer and bind it
		for (U32 i = 0; i < job.pipeline.constCount; ++i)
			dynamicBuffer.AllocBind(dev, cl, bindCtx, job.cbs[i].data, job.cbs[i].num32BitEntries * sizeof(U32));

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
#pragma endregion
}