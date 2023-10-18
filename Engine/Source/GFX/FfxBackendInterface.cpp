#include "GFX/FfxBackendInterface.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/FfxEffects.h"
#include "Data/Library.h"

// Assert for FFX backend interface pointer
#define ZE_CHECK_FFX_BACKEND() ZE_ASSERT(backendInterface, "Empty FFX backend interface!")

namespace ZE::GFX
{
	// Custom name for the resource
	struct FfxResourceName
	{
		wchar_t Name[FFX_RESOURCE_NAME_SIZE];
	};
	// Data about current and starting resource state
	struct FfxResourceStateInfo
	{
		Resource::State Current;
		Resource::State Initial;
	};
	// Tag for resources registered per frame from outside
	struct FfxDynamicResource {};
	// Handle for resources used by FfxBackendContext
	typedef U32 ResID;

	// Interface data setup when filling FfxInterface
	struct FfxBackendInterface
	{
		Device& Dev;
		ChainPool<Resource::DynamicCBuffer>& DynamicBuffers;
		U32 ContextRefCount = 0;
	};

	// Main context used by FFX SDK
	struct FfxBackendContext
	{
		entt::basic_registry<ResID> Resources;
		Data::Library<U64, Resource::PipelineStateCompute> Pipelines;
		Data::Library<U64, U64> PipelinesReferences;
		Data::Library<U64, Binding::Schema> Bindings;
		Data::Library<U64, U64> BindingsReferences;
		Data::Library<IndirectCommandType, CommandSignature> CommandSignatures;
		Data::Library<IndirectCommandType, U64> CommandSignaturesReferences;
		std::vector<Resource::GenericResourceBarrier> Barriers;
		std::vector<FfxGpuJobDescription> Jobs;
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
	constexpr ResID GetResID(S32 internalIndex) noexcept { ZE_ASSERT(internalIndex, "Invalid FFX resource index"); return internalIndex - 1; }
	constexpr S32 GetInternalIndex(ResID id) noexcept { return id + 1; }
	constexpr FfxBackendContext& GetFfxCtx(FfxInterface* backendInterface) noexcept;
	constexpr Device& GetDevice(FfxInterface* backendInterface) noexcept;
	constexpr Resource::DynamicCBuffer& GetDynamicBuffer(FfxInterface* backendInterface) noexcept;
	constexpr U32& GetFfxCtxRefCount(FfxInterface* backendInterface) noexcept;
	constexpr CommandList& GetCommandList(FfxCommandList commandList) noexcept;
	constexpr Resource::GenericResourceType GetResourceType(FfxResourceType type) noexcept;
	constexpr Resource::GenericResourceHeap GetHeapType(FfxHeapType type) noexcept;
	constexpr Resource::GenericResourceFlags GetResourceFlags(FfxResourceUsage usage) noexcept;
	constexpr PixelFormat GetPixelFormat(FfxSurfaceFormat format) noexcept;
	constexpr FfxSurfaceFormat GetSurfaceFormat(PixelFormat format) noexcept;
	constexpr Resource::State GetState(FfxResourceStates state) noexcept;
	constexpr FfxResourceStates GetState(Resource::State state) noexcept;
	constexpr Resource::Texture::AddressMode GetAddressMode(FfxAddressMode mode) noexcept;
	constexpr Resource::SamplerFilter GetFilter(FfxFilterType filter) noexcept;
	void AddResourceBarrier(FfxBackendContext& ctx, ResID resId, Resource::State after) noexcept;
	void FlushBarriers(FfxBackendContext& ctx, Device& dev, CommandList& cl);
	void ExecuteClearJob(FfxBackendContext& ctx, Device& dev, CommandList& cl, const FfxClearFloatJobDescription& job);
	void ExecuteCopyJob(FfxBackendContext& ctx, Device& dev, CommandList& cl, const FfxCopyJobDescription& job);
	void ExecuteComputeJob(FfxBackendContext& ctx, Device& dev, CommandList& cl, Resource::DynamicCBuffer& dynamicBuffer, const FfxComputeJobDescription& job);

	FfxResource ffxGetResource(Pipeline::FrameBuffer& buffers, Resource::Generic& res, RID rid, Resource::State state) noexcept
	{
		// Create proxy resource
		res.Init(buffers, rid);

		FfxResource desc = {};
		desc.resource = &res;
		desc.description.type = buffers.IsCubeTexture(rid) ? FFX_RESOURCE_TYPE_TEXTURE_CUBE : FFX_RESOURCE_TYPE_TEXTURE2D;
		desc.description.format = GetSurfaceFormat(buffers.GetFormat(rid));
		auto sizes = buffers.GetDimmensions(rid);
		desc.description.width = sizes.X;
		desc.description.height = sizes.Y;
		desc.description.depth = buffers.GetArraySize(rid);
		desc.description.mipCount = buffers.GetMipCount(rid);
		desc.description.flags = FFX_RESOURCE_FLAGS_NONE;
		desc.description.usage = FFX_RESOURCE_USAGE_READ_ONLY;
		if (buffers.IsUAV(rid))
			desc.description.usage = static_cast<FfxResourceUsage>(desc.description.usage | FFX_RESOURCE_USAGE_UAV);
		if (buffers.IsArrayView(rid))
			desc.description.usage = static_cast<FfxResourceUsage>(desc.description.usage | FFX_RESOURCE_USAGE_ARRAYVIEW);
		desc.state = GetState(state);
		return desc;
	}

	void ffxGetInterface(Device& dev, ChainPool<Resource::DynamicCBuffer>& dynamicBuffers) noexcept
	{
		FfxInterface& backendInterface = dev.GetFfxInterface();
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

		// Memory assignments
		backendInterface.scratchBufferSize = sizeof(FfxBackendContext);
		backendInterface.scratchBuffer = new U8[backendInterface.scratchBufferSize];

		// Set the device and dynamic buffers
		backendInterface.device = new FfxBackendInterface{ dev, dynamicBuffers };
		// Set assert printing
		ffxAssertSetPrintingCallback(ffxAssertCallback);
	}

	void ffxDestroyInterface(Device& dev) noexcept
	{
		FfxInterface& backendInterface = dev.GetFfxInterface();
		if (backendInterface.device)
		{
			delete reinterpret_cast<FfxBackendInterface*>(backendInterface.device);
			backendInterface.device = nullptr;
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
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(backendInterface->scratchBuffer, "Empty FFX backend context memory!");

		// Not using effect IDs anyway
		if (effectContextId)
			*effectContextId = 0;

		if (GetFfxCtxRefCount(backendInterface)++ == 0)
		{
			FfxBackendContext* ctx = reinterpret_cast<FfxBackendContext*>(backendInterface->scratchBuffer);
			std::memset(ctx, 0, sizeof(FfxBackendContext));
			new(ctx) FfxBackendContext;
		}

		return FFX_OK;
	}

	FfxErrorCode ffxGetDeviceCapabilities(FfxInterface* backendInterface, FfxDeviceCapabilities* deviceCapabilities)
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

	FfxErrorCode ffxDestroyBackendContext(FfxInterface* backendInterface, FfxUInt32 effectContextId)
	{
		ZE_CHECK_FFX_BACKEND();

		if (--GetFfxCtxRefCount(backendInterface) == 0)
		{
			Device& dev = GetDevice(backendInterface);
			FfxBackendContext& ctx = GetFfxCtx(backendInterface);

			// Free all remaining resources
			for (ResID res : ctx.Resources.view<Resource::Generic>())
				ctx.Resources.get<Resource::Generic>(res).Free(dev);
			ctx.Resources.clear();

			ctx.Pipelines.Transform([&dev](Resource::PipelineStateCompute& pipeline) { pipeline.Free(dev); });
			ctx.Bindings.Transform([&dev](Binding::Schema& schema) { schema.Free(dev); });
			ctx.CommandSignatures.Transform([&dev](CommandSignature& signature) { signature.Free(dev); });

			ctx.~FfxBackendContext();
		}
		return FFX_OK;
	}

	FfxErrorCode ffxCreateResource(FfxInterface* backendInterface,
		const FfxCreateResourceDescription* createResourceDescription,
		FfxUInt32 effectContextId, FfxResourceInternal* outTexture)
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(createResourceDescription, "Empty FFX resource description");
		ZE_ASSERT(outTexture, "Empty FFX out texture!");

		// FFX_RESOURCE_FLAGS_ALIASABLE -> make0 use of it somewhere here or rework Framebuffer to make use of this resource
		// FFX_RESOURCE_FLAGS_UNDEFINED -> only for Vulkan meaning that there is no source data and first barrier must provide layout as undefined (maybe for new DX12 barriers too)

		Device& dev = GetDevice(backendInterface);
		FfxBackendContext& ctx = GetFfxCtx(backendInterface);

		const ResID id = ctx.Resources.create();
		outTexture->internalIndex = GetInternalIndex(id);

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
		resDesc.DepthOrArraySize = Utils::SafeCast<U16>(createResourceDescription->resourceDescription.depth);
		resDesc.InitState = GetState(createResourceDescription->initalState);
		resDesc.InitDataSize = createResourceDescription->initDataSize;
		resDesc.InitData = createResourceDescription->initData;
		ZE_GEN_RES_SET_NAME(resDesc, Utils::ToUTF8(createResourceDescription->name));

		ctx.Resources.emplace<Resource::Generic>(id, dev, resDesc);
		ctx.Resources.emplace<FfxResourceStateInfo>(id, resDesc.InitState, resDesc.InitState);
		ctx.Resources.emplace<FfxResourceDescription>(id, createResourceDescription->resourceDescription);
#if _ZE_DEBUG_GFX_NAMES
		if (createResourceDescription->name)
			wcscpy_s(ctx.Resources.emplace<FfxResourceName>(id).Name, createResourceDescription->name);
#endif
		return FFX_OK;
	}

	FfxErrorCode ffxDestroyResource(FfxInterface* backendInterface, FfxResourceInternal resource)
	{
		ZE_CHECK_FFX_BACKEND();

		if (resource.internalIndex)
		{
			FfxBackendContext& ctx = GetFfxCtx(backendInterface);
			const ResID id = GetResID(resource.internalIndex);

			ctx.Resources.get<Resource::Generic>(id).Free(GetDevice(backendInterface));
			ctx.Resources.destroy(id);
		}
		return FFX_OK;
	}

	FfxResource ffxGetResource(FfxInterface* backendInterface, FfxResourceInternal resource)
	{
		ZE_CHECK_FFX_BACKEND();

		FfxBackendContext& ctx = GetFfxCtx(backendInterface);
		const ResID id = GetResID(resource.internalIndex);

		FfxResource res = {};
		res.resource = reinterpret_cast<void*>(&ctx.Resources.get<Resource::Generic>(id));
		res.state = GetState(ctx.Resources.get<FfxResourceStateInfo>(id).Current);
		res.description = ffxGetResourceDescriptor(backendInterface, resource);
#if _ZE_DEBUG_GFX_NAMES
		if (FfxResourceName* name = ctx.Resources.try_get<FfxResourceName>(id))
			wcscpy_s(res.name, name->Name);
#endif
		return res;
	}

	FfxErrorCode ffxRegisterResource(FfxInterface* backendInterface, const FfxResource* inResource,
		FfxUInt32 effectContextId, FfxResourceInternal* outResourceInternal)
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(inResource, "Empty FFX input resource!");
		ZE_ASSERT(outResourceInternal, "Empty FFX out resource!");

		if (inResource->resource)
		{
			FfxBackendContext& ctx = GetFfxCtx(backendInterface);
			const ResID id = ctx.Resources.create();
			outResourceInternal->internalIndex = GetInternalIndex(id);

			Resource::Generic& res = *reinterpret_cast<Resource::Generic*>(inResource->resource);
			ctx.Resources.emplace<Resource::Generic>(id, std::move(res));
			res.Free(GetDevice(backendInterface)); // Free resource as it's only proxy one

			const Resource::State state = GetState(inResource->state);
			ctx.Resources.emplace<FfxResourceStateInfo>(id, state, state);
			ctx.Resources.emplace<FfxResourceDescription>(id, inResource->description);
#if _ZE_DEBUG_GFX_NAMES
			if (inResource->name)
				wcscpy_s(ctx.Resources.emplace<FfxResourceName>(id).Name, inResource->name);
#endif
			ctx.Resources.emplace<FfxDynamicResource>(id); // Tag as dynamic per-frame resource
		}
		else
			outResourceInternal->internalIndex = 0;
		return FFX_OK;
	}

	FfxErrorCode ffxUnregisterResources(FfxInterface* backendInterface, FfxCommandList commandList, FfxUInt32 effectContextId)
	{
		ZE_CHECK_FFX_BACKEND();

		FfxBackendContext& ctx = GetFfxCtx(backendInterface);
		Device& dev = GetDevice(backendInterface);

		// Walk back all the resources that don't belong to FFX and reset them to their initial state
		for (ResID res : ctx.Resources.view<FfxDynamicResource>())
			AddResourceBarrier(ctx, res, ctx.Resources.get<FfxResourceStateInfo>(res).Initial);
		FlushBarriers(ctx, dev, GetCommandList(commandList));

		// Clear dynamic resources
		for (ResID res : ctx.Resources.view<FfxDynamicResource>())
			ctx.Resources.get<Resource::Generic>(res).Free(dev);
		ctx.Resources.destroy(ctx.Resources.view<FfxDynamicResource>().begin(), ctx.Resources.view<FfxDynamicResource>().end());
		return FFX_OK;
	}

	FfxResourceDescription ffxGetResourceDescriptor(FfxInterface* backendInterface, FfxResourceInternal resource)
	{
		ZE_CHECK_FFX_BACKEND();

		return GetFfxCtx(backendInterface).Resources.get<FfxResourceDescription>(GetResID(resource.internalIndex));
	}

	FfxErrorCode ffxCreatePipeline(FfxInterface* backendInterface, FfxEffect effect, FfxPass passId,
		uint32_t permutationOptions, const FfxPipelineDescription* desc, FfxUInt32 effectContextId, FfxPipelineState* outPipeline)
	{
		ZE_CHECK_FFX_BACKEND();
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
			U32 slotOffset = 0;
			U8 rangeOffset = 0;
			ZE_ASSERT(rangeOffset + shaderBlob.uavBufferCount <= UINT8_MAX, "Buffers outside possible range!");
			for (U32 i = 0; i < shaderBlob.uavBufferCount; ++i)
			{
				schemaDesc.AddRange({ shaderBlob.boundUAVBufferCounts[i], slotOffset, rangeOffset,
					Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack });

				slotOffset += shaderBlob.boundUAVBufferCounts[i];
				++rangeOffset;
			}

			ZE_ASSERT(rangeOffset + shaderBlob.uavTextureCount <= UINT8_MAX, "Buffers outside possible range!");
			for (U32 i = 0; i < shaderBlob.uavTextureCount; ++i)
			{
				schemaDesc.AddRange({ shaderBlob.boundUAVTextureCounts[i], slotOffset, rangeOffset,
					Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack });

				slotOffset += shaderBlob.boundUAVTextureCounts[i];
				++rangeOffset;
			}

			slotOffset = 0;
			ZE_ASSERT(rangeOffset + shaderBlob.srvBufferCount <= UINT8_MAX, "Buffers outside possible range!");
			for (U32 i = 0; i < shaderBlob.srvBufferCount; ++i)
			{
				schemaDesc.AddRange({ shaderBlob.boundSRVBufferCounts[i], slotOffset, rangeOffset,
					Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });

				slotOffset += shaderBlob.boundSRVBufferCounts[i];
				++rangeOffset;
			}

			ZE_ASSERT(rangeOffset + shaderBlob.srvTextureCount <= UINT8_MAX, "Buffers outside possible range!");
			for (U32 i = 0; i < shaderBlob.srvTextureCount; ++i)
			{
				schemaDesc.AddRange({ shaderBlob.boundSRVTextureCounts[i], slotOffset, rangeOffset,
					Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });

				slotOffset += shaderBlob.boundSRVTextureCounts[i];
				++rangeOffset;
			}

			slotOffset = 0;
			for (U32 i = 0; i < shaderBlob.cbvCount; ++i)
			{
				schemaDesc.AddRange({ shaderBlob.boundConstantBufferCounts[i], slotOffset, rangeOffset,
					Resource::ShaderType::Compute, Binding::RangeFlag::CBV });

				slotOffset += shaderBlob.boundConstantBufferCounts[i];
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
		ZE_CHECK_FFX_BACKEND();

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
		ZE_CHECK_FFX_BACKEND();
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
		ZE_CHECK_FFX_BACKEND();

		FfxBackendContext& ctx = GetFfxCtx(backendInterface);
		Device& dev = GetDevice(backendInterface);
		Resource::DynamicCBuffer& dynamicBuffer = GetDynamicBuffer(backendInterface);
		CommandList& cl = GetCommandList(commandList);

		for (const FfxGpuJobDescription& job : ctx.Jobs)
		{
			switch (job.jobType)
			{
			case FFX_GPU_JOB_CLEAR_FLOAT:
				ExecuteClearJob(ctx, dev, cl, job.clearJobDescriptor);
				break;
			case FFX_GPU_JOB_COPY:
				ExecuteCopyJob(ctx, dev, cl, job.copyJobDescriptor);
				break;
			case FFX_GPU_JOB_COMPUTE:
				ExecuteComputeJob(ctx, dev, cl, dynamicBuffer, job.computeJobDescriptor);
				break;
			default:
				ZE_FAIL("Unknown FFX GPU job!");
				break;
			}
		}
		ctx.Jobs.clear();

		return FFX_OK;
	}
#pragma endregion
#pragma region FFX utility functions
	constexpr FfxBackendContext& GetFfxCtx(FfxInterface* backendInterface) noexcept
	{
		ZE_ASSERT(backendInterface->scratchBuffer, "Empty FFX backend context!");
		return *((FfxBackendContext*)backendInterface->scratchBuffer);
	}

	constexpr Device& GetDevice(FfxInterface* backendInterface) noexcept
	{
		ZE_ASSERT(backendInterface->device, "Empty FFX device interface!");
		return ((FfxBackendInterface*)backendInterface->device)->Dev;
	}

	constexpr Resource::DynamicCBuffer& GetDynamicBuffer(FfxInterface* backendInterface) noexcept
	{
		ZE_ASSERT(backendInterface->device, "Empty FFX device interface!");
		return ((FfxBackendInterface*)backendInterface->device)->DynamicBuffers.Get();
	}

	constexpr U32& GetFfxCtxRefCount(FfxInterface* backendInterface) noexcept
	{
		ZE_ASSERT(backendInterface->device, "Empty FFX device interface!");
		return ((FfxBackendInterface*)backendInterface->device)->ContextRefCount;
	}

	constexpr CommandList& GetCommandList(FfxCommandList commandList) noexcept
	{
		ZE_ASSERT(commandList, "Empty FFX command list!");
		return *((CommandList*)commandList);
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

	constexpr FfxSurfaceFormat GetSurfaceFormat(PixelFormat format) noexcept
	{
		switch (format)
		{
		default:
			ZE_FAIL("Format not yet supported by FidelityFX SDK!");
			[[fallthrough]];
		case PixelFormat::Unknown:
			return FFX_SURFACE_FORMAT_UNKNOWN;
		case PixelFormat::R32G32B32A32_UInt:
			return FFX_SURFACE_FORMAT_R32G32B32A32_UINT;
		case PixelFormat::R32G32B32A32_Float:
			return FFX_SURFACE_FORMAT_R32G32B32A32_FLOAT;
		case PixelFormat::R16G16B16A16_Float:
			return FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT;
		case PixelFormat::R32G32_Float:
			return FFX_SURFACE_FORMAT_R32G32_FLOAT;
		case PixelFormat::R8_UInt:
			return FFX_SURFACE_FORMAT_R8_UINT;
		case PixelFormat::R32_UInt:
			return FFX_SURFACE_FORMAT_R32_UINT;
		case PixelFormat::R8G8B8A8_UInt:
		case PixelFormat::R8G8B8A8_SInt:
			ZE_WARNING("FidelityFX SDK is not supporting plain R8G8B8A8_UInt so falling back to typeless version!");
			return FFX_SURFACE_FORMAT_R8G8B8A8_TYPELESS;
		case PixelFormat::R8G8B8A8_UNorm:
			return FFX_SURFACE_FORMAT_R8G8B8A8_UNORM;
		case PixelFormat::R8G8B8A8_SNorm:
			return FFX_SURFACE_FORMAT_R8G8B8A8_SNORM;
		case PixelFormat::R8G8B8A8_UNorm_SRGB:
			return FFX_SURFACE_FORMAT_R8G8B8A8_SRGB;
		case PixelFormat::R11G11B10_Float:
			return FFX_SURFACE_FORMAT_R11G11B10_FLOAT;
		case PixelFormat::R16G16_Float:
			return FFX_SURFACE_FORMAT_R16G16_FLOAT;
		case PixelFormat::R16G16_UInt:
			return FFX_SURFACE_FORMAT_R16G16_UINT;
		case PixelFormat::R16_Float:
		case PixelFormat::R16_Depth:
			return FFX_SURFACE_FORMAT_R16_FLOAT;
		case PixelFormat::R16_UInt:
			return FFX_SURFACE_FORMAT_R16_UINT;
		case PixelFormat::R16_UNorm:
			return FFX_SURFACE_FORMAT_R16_UNORM;
		case PixelFormat::R16_SNorm:
			return FFX_SURFACE_FORMAT_R16_SNORM;
		case PixelFormat::R8_UNorm:
			return FFX_SURFACE_FORMAT_R8_UNORM;
		case PixelFormat::R8G8_UNorm:
			return FFX_SURFACE_FORMAT_R8G8_UNORM;
		case PixelFormat::R32_Float:
		case PixelFormat::R32_Depth:
			return FFX_SURFACE_FORMAT_R32_FLOAT;
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
			return PixelFormat::R16G16_UInt;
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

	constexpr FfxResourceStates GetState(Resource::State state) noexcept
	{
		switch (state)
		{
		case Resource::State::StateUnorderedAccess:
			return FFX_RESOURCE_STATE_UNORDERED_ACCESS;
		case Resource::State::StateShaderResourceNonPS:
			return FFX_RESOURCE_STATE_COMPUTE_READ;
		case Resource::State::StateShaderResourcePS:
			return FFX_RESOURCE_STATE_PIXEL_READ;
		case Resource::State::StateShaderResourceAll:
			return FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ;
		case Resource::State::StateCopySource:
			return FFX_RESOURCE_STATE_COPY_SRC;
		case Resource::State::StateCopyDestination:
			return FFX_RESOURCE_STATE_COPY_DEST;
		case Resource::State::StateGenericRead:
			return FFX_RESOURCE_STATE_GENERIC_READ;
		case Resource::State::StateIndirect:
			return FFX_RESOURCE_STATE_INDIRECT_ARGUMENT;
		default:
		{
			ZE_FAIL("Unhandled resource state!");
			return FFX_RESOURCE_STATE_GENERIC_READ;
		}
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

	void AddResourceBarrier(FfxBackendContext& ctx, ResID resId, Resource::State after) noexcept
	{
		Resource::State& current = ctx.Resources.get<FfxResourceStateInfo>(resId).Current;
		if ((current & after) != after)
		{
			ctx.Barriers.emplace_back(false, &ctx.Resources.get<Resource::Generic>(resId), current, after);
			current = after;
		}
		else if (after == Resource::State::StateUnorderedAccess)
		{
			ctx.Barriers.emplace_back(true, &ctx.Resources.get<Resource::Generic>(resId), Resource::State::StateUnorderedAccess, Resource::State::StateUnorderedAccess);
		}
	}

	void FlushBarriers(FfxBackendContext& ctx, Device& dev, CommandList& cl)
	{
		if (ctx.Barriers.size())
		{
			cl.Barrier(dev, ctx.Barriers.data(), Utils::SafeCast<U32>(ctx.Barriers.size()));
			ctx.Barriers.clear();
		}
	}

	void ExecuteClearJob(FfxBackendContext& ctx, Device& dev, CommandList& cl, const FfxClearFloatJobDescription& job)
	{
		const ResID id = GetResID(job.target.internalIndex);
		AddResourceBarrier(ctx, id, Resource::State::StateUnorderedAccess);
		FlushBarriers(ctx, dev, cl);

		ctx.Resources.get<Resource::Generic>(id).ClearUAV(cl, *reinterpret_cast<const ColorF4*>(job.color));
	}

	void ExecuteCopyJob(FfxBackendContext& ctx, Device& dev, CommandList& cl, const FfxCopyJobDescription& job)
	{
		const ResID srcId = GetResID(job.src.internalIndex);
		const ResID destId = GetResID(job.dst.internalIndex);
		AddResourceBarrier(ctx, srcId, Resource::State::StateCopySource);
		AddResourceBarrier(ctx, destId, Resource::State::StateCopyDestination);
		FlushBarriers(ctx, dev, cl);

		ctx.Resources.get<Resource::Generic>(srcId).Copy(dev, cl, ctx.Resources.get<Resource::Generic>(destId));
	}

	void ExecuteComputeJob(FfxBackendContext& ctx, Device& dev, CommandList& cl, Resource::DynamicCBuffer& dynamicBuffer, const FfxComputeJobDescription& job)
	{
		// Transition all the UAVs and SRVs
		for (U32 i = 0; i < job.pipeline.uavBufferCount; ++i)
			if (job.uavBuffers[i].internalIndex)
				AddResourceBarrier(ctx, GetResID(job.uavBuffers[i].internalIndex), Resource::State::StateUnorderedAccess);
		for (U32 i = 0; i < job.pipeline.uavTextureCount; ++i)
			if (job.uavTextures[i].internalIndex)
				AddResourceBarrier(ctx, GetResID(job.uavTextures[i].internalIndex), Resource::State::StateUnorderedAccess);
		for (U32 i = 0; i < job.pipeline.srvBufferCount; ++i)
			if (job.srvBuffers[i].internalIndex)
				AddResourceBarrier(ctx, GetResID(job.srvBuffers[i].internalIndex), Resource::State::StateShaderResourceNonPS);
		for (U32 i = 0; i < job.pipeline.srvTextureCount; ++i)
			if (job.srvTextures[i].internalIndex)
				AddResourceBarrier(ctx, GetResID(job.srvTextures[i].internalIndex), Resource::State::StateShaderResourceNonPS);

		// If we are dispatching indirectly, transition the argument resource to indirect argument
		if (job.pipeline.cmdSignature)
			AddResourceBarrier(ctx, GetResID(job.cmdArgument.internalIndex), Resource::State::StateIndirect);
		FlushBarriers(ctx, dev, cl);

		// Bind pipeline with binding schema
		ctx.Pipelines.Get(reinterpret_cast<U64>(job.pipeline.pipeline)).Bind(cl);
		Binding::Context bindCtx = { ctx.Bindings.Get(reinterpret_cast<U64>(job.pipeline.rootSignature)) };
		bindCtx.BindingSchema.SetCompute(cl);

		// Bind all resources
		for (U32 i = 0; i < job.pipeline.uavBufferCount; ++i)
		{
			if (job.uavBuffers[i].internalIndex)
				ctx.Resources.get<Resource::Generic>(GetResID(job.uavBuffers[i].internalIndex)).Bind(dev, cl, bindCtx, true);
			else
				++bindCtx.Count;
		}
		for (U32 i = 0; i < job.pipeline.uavTextureCount; ++i)
		{
			if (job.uavTextures[i].internalIndex)
				ctx.Resources.get<Resource::Generic>(GetResID(job.uavTextures[i].internalIndex)).Bind(dev, cl, bindCtx, true, Utils::SafeCast<U16>(job.uavTextureMips[i]));
			else
				++bindCtx.Count;
		}
		for (U32 i = 0; i < job.pipeline.srvBufferCount; ++i)
		{
			if (job.srvBuffers[i].internalIndex)
				ctx.Resources.get<Resource::Generic>(GetResID(job.srvBuffers[i].internalIndex)).Bind(dev, cl, bindCtx, false);
			else
				++bindCtx.Count;
		}
		for (U32 i = 0; i < job.pipeline.srvTextureCount; ++i)
		{
			if (job.srvTextures[i].internalIndex)
				ctx.Resources.get<Resource::Generic>(GetResID(job.srvTextures[i].internalIndex)).Bind(dev, cl, bindCtx, false);
			else
				++bindCtx.Count;
		}

		// Copy data to dynamic cbuffer and bind it
		for (U32 i = 0; i < job.pipeline.constCount; ++i)
			dynamicBuffer.AllocBind(dev, cl, bindCtx, job.cbs[i].data, job.cbs[i].num32BitEntries * sizeof(U32));

		// Dispatch (or dispatch indirect)
		if (job.pipeline.cmdSignature)
		{
			ctx.Resources.get<Resource::Generic>(GetResID(job.cmdArgument.internalIndex)).ExecuteIndirectCommands(cl,
				ctx.CommandSignatures.Get(static_cast<IndirectCommandType>(reinterpret_cast<U64>(job.pipeline.cmdSignature))), job.cmdArgumentOffset);
		}
		else
			cl.Compute(dev, job.dimensions[0], job.dimensions[1], job.dimensions[2]);
	}
#pragma endregion
}