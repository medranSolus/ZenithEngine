#include "GFX/FfxBackendInterface.h"
#include "GFX/Binding/Schema.h"
#include "GFX/Resource/Generic.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/Shader.h"
#include "GFX/CommandSignature.h"
#include "Data/Library.h"

// Assert for FFX backend interface pointer
#define ZE_CHECK_FFX_BACKEND() ZE_ASSERT(backendInterface, "Empty FFX backend interface!")

namespace ZE::GFX
{
	struct FfxResourceName
	{
		wchar_t Name[FFX_RESOURCE_NAME_SIZE];
	};
	struct FfxDynamicResource {};

	struct FfxBackendContext
	{
		entt::basic_registry<U32> Resources;
		Data::Library<U64, Resource::PipelineStateCompute> Pipelines;
		Data::Library<U64, Binding::Schema> Bindings;
		Data::Library<IndirectCommandType, CommandSignature> CommandSignatures;
		std::vector<Resource::GenericResourceBarrier> Barriers;
	};

	// Interface functions used by FFX SDK backend
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
	constexpr FfxBackendContext& GetFfxCtx(FfxInterface* backendInterface) noexcept;
	constexpr Device& GetDevice(FfxInterface* backendInterface) noexcept;
	constexpr CommandList& GetCommandList(FfxCommandList commandList) noexcept;
	constexpr Resource::GenericResourceType GetResourceType(FfxResourceType type) noexcept;
	constexpr Resource::GenericResourceHeap GetHeapType(FfxHeapType type) noexcept;
	constexpr Resource::GenericResourceFlags GetResourceFlags(FfxResourceUsage usage) noexcept;
	constexpr PixelFormat GetPixelFormat(FfxSurfaceFormat format) noexcept;
	constexpr Resource::State GetState(FfxResourceStates state) noexcept;
	constexpr Resource::Texture::AddressMode GetAddressMode(FfxAddressMode mode) noexcept;
	constexpr Resource::SamplerFilter GetFilter(FfxFilterType filter) noexcept;
	constexpr U64 GetPipelineID(FfxEffect effect, FfxPass passId, U32 permutationOptions) noexcept;

	void ffxGetInterface(FfxInterface& backendInterface, Device& dev) noexcept
	{
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
		//backendInterface->fpScheduleGpuJob = ScheduleGpuJobDX12;
		//backendInterface->fpExecuteGpuJobs = ExecuteGpuJobsDX12;

		// Memory assignments
		backendInterface.scratchBufferSize = sizeof(FfxBackendContext);
		backendInterface.scratchBuffer = new U8[backendInterface.scratchBufferSize];

		// Set the device
		backendInterface.device = ffxGetDevice(dev);
	}

#pragma region FFX backend functions
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

		FfxBackendContext* ctx = reinterpret_cast<FfxBackendContext*>(backendInterface->scratchBuffer);
		std::memset(ctx, 0, sizeof(FfxBackendContext));
		new(ctx) FfxBackendContext;

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
		ZE_ASSERT(backendInterface, "Empty FFX backend interface!");

		Device& dev = GetDevice(backendInterface);
		FfxBackendContext& ctx = GetFfxCtx(backendInterface);

		// Free all remaining resources
		for (S32 res : ctx.Resources.view<Resource::Generic>())
			ctx.Resources.get<Resource::Generic>(res).Free(dev);
		ctx.Resources.clear();

		ctx.Pipelines.Transform([&dev](Resource::PipelineStateCompute& pipeline) { pipeline.Free(dev); });
		ctx.Bindings.Transform([&dev](Binding::Schema& schema) { schema.Free(dev); });
		ctx.CommandSignatures.Transform([&dev](CommandSignature& signature) { signature.Free(dev); });

		ctx.~FfxBackendContext();
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

		outTexture->internalIndex = ctx.Resources.create();

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

		ctx.Resources.emplace<Resource::Generic>(outTexture->internalIndex, dev, resDesc);
		ctx.Resources.emplace<FfxResourceStates>(outTexture->internalIndex, createResourceDescription->initalState); // Current state
		ctx.Resources.emplace<Resource::State>(outTexture->internalIndex, resDesc.InitState); // Initial state (read-only)
		ctx.Resources.emplace<FfxResourceDescription>(outTexture->internalIndex, createResourceDescription->resourceDescription);
#if _ZE_DEBUG_GFX_NAMES
		if (createResourceDescription->name)
			wcscpy_s(ctx.Resources.emplace<FfxResourceName>(outTexture->internalIndex).Name, createResourceDescription->name);
#endif
		return FFX_OK;
	}

	FfxErrorCode ffxDestroyResource(FfxInterface* backendInterface, FfxResourceInternal resource)
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(resource.internalIndex, "Invalid FFX resource index");

		FfxBackendContext& ctx = GetFfxCtx(backendInterface);
		ctx.Resources.get<Resource::Generic>(resource.internalIndex).Free(GetDevice(backendInterface));
		ctx.Resources.destroy(resource.internalIndex);

		return FFX_OK;
	}

	FfxResource ffxGetResource(FfxInterface* backendInterface, FfxResourceInternal resource)
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(resource.internalIndex, "Invalid FFX resource index");

		FfxBackendContext& ctx = GetFfxCtx(backendInterface);

		FfxResource res = {};
		res.resource = reinterpret_cast<void*>(&ctx.Resources.get<Resource::Generic>(resource.internalIndex));
		res.state = ctx.Resources.get<FfxResourceStates>(resource.internalIndex);
		res.description = ffxGetResourceDescriptor(backendInterface, resource);
#if _ZE_DEBUG_GFX_NAMES
		if (FfxResourceName* name = ctx.Resources.try_get<FfxResourceName>(resource.internalIndex))
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
			outResourceInternal->internalIndex = ctx.Resources.create();

			ctx.Resources.emplace<Resource::Generic>(outResourceInternal->internalIndex, std::move(*reinterpret_cast<Resource::Generic*>(inResource->resource)));
			ctx.Resources.emplace<FfxResourceStates>(outResourceInternal->internalIndex, inResource->state); // Current state
			ctx.Resources.emplace<Resource::State>(outResourceInternal->internalIndex, GetState(inResource->state)); // Initial state (read-only)
			ctx.Resources.emplace<FfxResourceDescription>(outResourceInternal->internalIndex, inResource->description);
#if _ZE_DEBUG_GFX_NAMES
			if (inResource->name)
				wcscpy_s(ctx.Resources.emplace<FfxResourceName>(outResourceInternal->internalIndex).Name, inResource->name);
#endif
			ctx.Resources.emplace<FfxDynamicResource>(outResourceInternal->internalIndex); // Tag as dynamic per-frame resource
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
		for (S32 res : ctx.Resources.view<FfxDynamicResource>())
		{
			ctx.Barriers.emplace_back(false, &ctx.Resources.get<Resource::Generic>(res),
				GetState(ctx.Resources.get<FfxResourceStates>(res)), // Current state
				ctx.Resources.get<Resource::State>(res)); // Initial state
		}

		if (ctx.Barriers.size())
			GetCommandList(commandList).Barrier(dev, ctx.Barriers.data(), Utils::SafeCast<U32>(ctx.Barriers.size()));

		// Clear dynamic resources
		for (S32 res : ctx.Resources.view<FfxDynamicResource>())
			ctx.Resources.get<Resource::Generic>(res).Free(dev);
		ctx.Resources.destroy(ctx.Resources.view<FfxDynamicResource>().begin(), ctx.Resources.view<FfxDynamicResource>().end());
		return FFX_OK;
	}

	FfxResourceDescription ffxGetResourceDescriptor(FfxInterface* backendInterface, FfxResourceInternal resource)
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(resource.internalIndex, "Invalid FFX resource index");

		return GetFfxCtx(backendInterface).Resources.get<FfxResourceDescription>(resource.internalIndex);
	}

	FfxErrorCode ffxCreatePipeline(FfxInterface* backendInterface, FfxEffect effect, FfxPass passId,
		uint32_t permutationOptions, const FfxPipelineDescription* desc, FfxUInt32 effectContextId, FfxPipelineState* outPipeline)
	{
		ZE_CHECK_FFX_BACKEND();
		ZE_ASSERT(desc, "Empty FFX pipeline desc!");
		ZE_ASSERT(outPipeline, "Empty FFX pipeline state!");
		ZE_ASSERT(desc->stage == FFX_BIND_COMPUTE_SHADER_STAGE, "Pipeline not for the compute shader!");

		Device& dev = GetDevice(backendInterface);
		FfxBackendContext& ctx = GetFfxCtx(backendInterface);
		const U64 id = GetPipelineID(effect, passId, permutationOptions);

		// Fill out for every shader that is created when needed by effect
		FfxShaderBlob shaderBlob = {};
		Resource::Shader shader;
		switch (effect)
		{
		default:
			ZE_FAIL("Selected effect has not been implemented yet!");
			return FFX_ERROR_INVALID_ENUM;
		case FFX_EFFECT_CACAO:
		{
			shader.Init(dev, "name");
			//ffxGetPermutationBlobByIndex(effect, passId, permutationOptions, &shaderBlob);

			break;
		}
		}

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

		// Set bindings
		if (shaderBlob.uavBufferCount || shaderBlob.uavTextureCount)
			schemaDesc.AddRange({ shaderBlob.uavBufferCount + shaderBlob.uavTextureCount, 0, 0, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack });
		if (shaderBlob.srvBufferCount || shaderBlob.srvTextureCount)
			schemaDesc.AddRange({ shaderBlob.srvBufferCount + shaderBlob.srvTextureCount, 0, static_cast<U8>(schemaDesc.Ranges.size()), Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		for (U32 i = 0; i < desc->rootConstantBufferCount; ++i)
		{
			ZE_ASSERT(desc->rootConstants[i].stage == FFX_BIND_COMPUTE_SHADER_STAGE, "Binding not for the compute shader!");
			schemaDesc.AddRange({ desc->rootConstants[i].size, i, static_cast<U8>(schemaDesc.Ranges.size()), Resource::ShaderType::Compute, Binding::RangeFlag::CBV | Binding::RangeFlag::Constant });
		}
		// TODO: cache binding based on input parameters
		ctx.Bindings.Add(id, dev, schemaDesc);
		outPipeline->rootSignature = reinterpret_cast<FfxPipeline>(id);

		// Only set the command signature if this is setup as an indirect workload
		if (desc->indirectWorkload)
		{
			if (!ctx.CommandSignatures.Contains(IndirectCommandType::Dispatch))
				ctx.CommandSignatures.Add(IndirectCommandType::Dispatch, dev, IndirectCommandType::Dispatch);
			outPipeline->cmdSignature = reinterpret_cast<FfxCommandSignature>(IndirectCommandType::Dispatch);
		}
		else
			outPipeline->cmdSignature = nullptr;

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

		// Create pipeline
		ctx.Pipelines.Add(id, dev, shader, ctx.Bindings.Get(id));
		outPipeline->pipeline = reinterpret_cast<FfxPipeline>(id);

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
				ctx.Bindings.Get(reinterpret_cast<U64>(pipeline->rootSignature)).Free(dev);
				ctx.Bindings.Remove(reinterpret_cast<U64>(pipeline->rootSignature));
				pipeline->rootSignature = nullptr;
			}
			pipeline->cmdSignature = nullptr;
			if (pipeline->pipeline)
			{
				ctx.Pipelines.Get(reinterpret_cast<U64>(pipeline->pipeline)).Free(dev);
				ctx.Pipelines.Remove(reinterpret_cast<U64>(pipeline->pipeline));
				pipeline->pipeline = nullptr;
			}
		}
		return FFX_OK;
	}

	FfxErrorCode ffxExecuteGpuJobs(FfxInterface* backendInterface, FfxCommandList commandList)
	{
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
		ZE_ASSERT(backendInterface->device, "Empty FFX device!");
		return *((Device*)backendInterface->device);
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

	constexpr U64 GetPipelineID(FfxEffect effect, FfxPass passId, U32 permutationOptions) noexcept
	{
		ZE_ASSERT(effect <= UINT8_MAX, "FFX effect id outside of cast range!");
		ZE_ASSERT(passId <= UINT8_MAX, "FFX pass id outside of cast range!");
		return static_cast<U64>(effect) | (static_cast<U64>(passId) << 8) | (static_cast<U64>(permutationOptions) << 16);
	}
#pragma endregion
}