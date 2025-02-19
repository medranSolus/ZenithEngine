#include "RHI/DX12/Device.h"
#include "RHI/DX12/DREDRecovery.h"
#include "GFX/CommandList.h"
#include "GFX/XeSSException.h"
#include "Data/Camera.h"
ZE_WARNING_PUSH
#include "ffx_api/dx12/ffx_api_dx12.h"
ZE_WARNING_POP

namespace ZE::RHI::DX12
{
	void Device::DescHeap::Init(DescHeap& chunk, Allocator::TLSFMemoryChunkFlags flags, U64 size, void* userData)
	{
		ZE_ASSERT(userData, "Empty device data!");
		Device& dev = *reinterpret_cast<Device*>(userData);
		ZE_DX_ENABLE(dev);

		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
		descHeapDesc.NodeMask = 0;
		descHeapDesc.Flags = static_cast<D3D12_DESCRIPTOR_HEAP_FLAGS>(flags);
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descHeapDesc.NumDescriptors = Utils::SafeCast<U32>(size);
		ZE_DX_THROW_FAILED(dev.GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&chunk.Heap)));
	}

	void Device::WaitCPU(IFence* fence, U64 val)
	{
		ZE_WIN_ENABLE_EXCEPT();

		if (fence->GetCompletedValue() < val)
		{
			HANDLE fenceEvent = CreateEventW(nullptr, false, false, nullptr);
			ZE_ASSERT(fenceEvent, "Cannot create fence event!");

			ZE_DX_THROW_FAILED(fence->SetEventOnCompletion(val, fenceEvent));
			if (WaitForSingleObject(fenceEvent, INFINITE) != WAIT_OBJECT_0)
				throw ZE_WIN_EXCEPT_LAST();
			if (CloseHandle(fenceEvent) == 0)
				throw ZE_WIN_EXCEPT_LAST();
		}
	}

	void Device::WaitGPU(IFence* fence, ICommandQueue* queue, U64 val)
	{
		ZE_WIN_ENABLE_EXCEPT();
		ZE_DX_THROW_FAILED(queue->Wait(fence, val));
	}

	U64 Device::SetFenceCPU(IFence* fence, UA64& fenceVal)
	{
		ZE_WIN_ENABLE_EXCEPT();
		U64 val = ++fenceVal;
		ZE_DX_THROW_FAILED(fence->Signal(val));
		return val;
	}

	U64 Device::SetFenceGPU(IFence* fence, ICommandQueue* queue, UA64& fenceVal)
	{
		ZE_WIN_ENABLE_EXCEPT();
		U64 val = ++fenceVal;
		ZE_DX_THROW_FAILED(queue->Signal(fence, val));
		return val;
	}

	void Device::Execute(ICommandQueue* queue, CommandList& cl)
	{
		ZE_ASSERT(cl.GetList() != nullptr, "Empty list!");
		ICommandList* lists[] = { cl.GetList() };
		ZE_DX_THROW_FAILED_INFO(queue->ExecuteCommandLists(1, lists));
	}

	Device::Device(const Window::MainWindow& window, U32 descriptorCount)
		: blockDescAllocator(BLOCK_DESCRIPTOR_ALLOC_CAPACITY), chunkDescAllocator(CHUNK_DESCRIPTOR_ALLOC_CAPACITY),
		descriptorGpuAllocator(blockDescAllocator, chunkDescAllocator, true),
		descriptorCpuAllocator(blockDescAllocator, chunkDescAllocator)
	{
		ZE_WIN_ENABLE_EXCEPT();
#if _ZE_DEBUG_GFX_NAMES
		std::string ZE_DX_DEBUG_ID;
#endif
		// No support for 8 bit indices on DirectX
		Settings::SetU8IndexBuffers(false);

#if !_ZE_MODE_RELEASE
		// Load WinPixGpuCapturer.dll
		if (Settings::IsEnabledPIXAttaching() && GetModuleHandleW(L"WinPixGpuCapturer.dll") == 0)
		{
			// Find latest WinPixGpuCapturer.dll path
			LPWSTR programFilesPath = nullptr;
			SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, nullptr, &programFilesPath);

			std::filesystem::path pixInstallationPath = programFilesPath;
			pixInstallationPath /= "Microsoft PIX";

			std::wstring newestVersionFound;
			for (const auto& entry : std::filesystem::directory_iterator(pixInstallationPath))
			{
				if (entry.is_directory())
				{
					if (newestVersionFound.empty() || newestVersionFound < entry.path().filename().c_str())
						newestVersionFound = entry.path().filename().c_str();
				}
			}

			if (newestVersionFound.empty())
				Logger::Warning("Cannot load requested \"WinPixGpuCapturer.dll\"!");
			else
			{
				pixCapturer = LoadLibraryW((pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll").c_str());
				if (pixCapturer == nullptr)
					Logger::Warning("Error loading \"WinPixGpuCapturer.dll\"! Error message:\n    " + WinAPI::WinApiException::TranslateErrorCode(GetLastError()));
			}
		}
#endif

#if _ZE_DEBUG_GFX_API
		// Enable Debug Layer before calling any DirectX commands
		DX::ComPtr<IDebug> debugInterface = nullptr;
		ZE_DX_THROW_FAILED_NOINFO(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();
		if (Settings::IsEnabledGPUValidation())
			debugInterface->SetEnableGPUBasedValidation(true);

		// Enable device removed recovery
		DREDRecovery::Enable(debugManager);
#endif

		DX::ComPtr<DX::IAdapter> adapter = DX::CreateAdapter(
#if _ZE_DEBUG_GFX_API
			debugManager
#endif
		);

		// Initialize via hardware specific functions
		switch (Settings::GpuVendor)
		{
		case GFX::VendorGPU::AMD:
		{
			AGSConfiguration agsConfig = {};
			if (agsInitialize(AGS_MAKE_VERSION(AMD_AGS_VERSION_MAJOR, AMD_AGS_VERSION_MINOR, AMD_AGS_VERSION_PATCH),
				&agsConfig, &gpuCtxAMD, nullptr) == AGS_SUCCESS)
			{
				AGSDX12DeviceCreationParams deviceParams = {};
				deviceParams.pAdapter = adapter.Get();
				deviceParams.iid = __uuidof(device);
				deviceParams.FeatureLevel = MINIMAL_D3D_LEVEL;

				AGSDX12ExtensionParams extensionParams = {};
				AGSDX12ReturnedParams returnParams;
				if (agsDriverExtensionsDX12_CreateDevice(gpuCtxAMD, &deviceParams, &extensionParams, &returnParams) == AGS_SUCCESS)
				{
					ZE_DX_THROW_FAILED(returnParams.pDevice->QueryInterface(IID_PPV_ARGS(&device)));
					returnParams.pDevice->Release();
					break;
				}
				agsDeInitialize(gpuCtxAMD);
				gpuCtxAMD = nullptr;
			}
			Settings::GpuVendor = GFX::VendorGPU::Unknown;
			break;
		}
		default:
			break;
		}

		// Failed to create GPU specific device
		if (device == nullptr)
		{
			ZE_DX_THROW_FAILED_NOINFO(D3D12CreateDevice(adapter.Get(), MINIMAL_D3D_LEVEL, IID_PPV_ARGS(&device)));
		}

		D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = {};
		ZE_WIN_THROW_FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12)));
		if (!options12.EnhancedBarriersSupported)
			throw ZE_CMP_EXCEPT("DX12 error: Enhanced Barriers not supported by current driver!");

#if _ZE_DEBUG_GFX_API
		DX::ComPtr<IInfoQueue> infoQueue = nullptr;
		ZE_DX_THROW_FAILED(device.As(&infoQueue));

		// Set breaks on dangerous messages
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

		// Suppress non important messages
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_MESSAGE_ID denyIds[] =
		{
			// D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			// Bug in Visual Studio Graphics Debugger while capturing frame
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
			// When asking for smaller alignment error is generated, silence it
			D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT,
			D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT_SMALLRESOURCE,
			// When performing initial upload of data from DirectStorage, barrier is required for proper initialization
			D3D12_MESSAGE_ID_NON_OPTIMAL_BARRIER_ONLY_EXECUTE_COMMAND_LISTS,
			// When DLSS is creating buffers with STATE_COPY_DESC while they can be set to STATE_COMMON since it doesn't make any difference
			D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED
		};

		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumSeverities = 1;
		filter.DenyList.pSeverityList = severities;
		filter.DenyList.NumIDs = sizeof(denyIds) / sizeof(D3D12_MESSAGE_ID);
		filter.DenyList.pIDList = denyIds;

		ZE_DX_THROW_FAILED(infoQueue->PushStorageFilter(&filter));

		if (Settings::IsEnabledGPUValidation())
		{
			DX::ComPtr<IDebugDevice> debugDevice = nullptr;
			ZE_DX_THROW_FAILED_NOINFO(device.As(&debugDevice));

			const D3D12_DEBUG_FEATURE debugFeature = D3D12_DEBUG_FEATURE_ALLOW_BEHAVIOR_CHANGING_DEBUG_AIDS;
			ZE_DX_THROW_FAILED(debugDevice->SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_FEATURE_FLAGS,
				&debugFeature, sizeof(D3D12_DEBUG_FEATURE)));

			D3D12_DEBUG_DEVICE_GPU_BASED_VALIDATION_SETTINGS validationSettings = {};
			// Should cover all messages
			validationSettings.MaxMessagesPerCommandList = 1024;
			// Can avoid most cases of TDRs
			validationSettings.DefaultShaderPatchMode = D3D12_GPU_BASED_VALIDATION_SHADER_PATCH_MODE_GUARDED_VALIDATION;
			validationSettings.PipelineStateCreateFlags = D3D12_GPU_BASED_VALIDATION_PIPELINE_STATE_CREATE_FLAG_FRONT_LOAD_CREATE_GUARDED_VALIDATION_SHADERS;
			ZE_DX_THROW_FAILED(debugDevice->SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_GPU_BASED_VALIDATION_SETTINGS,
				&validationSettings, sizeof(D3D12_DEBUG_DEVICE_GPU_BASED_VALIDATION_SETTINGS)));
		}
#endif
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ZE_DX_THROW_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&mainQueue)));
		ZE_DX_SET_ID(mainQueue, "direct_queue");
		ZE_DX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mainFence)));
		ZE_DX_SET_ID(mainFence, "direct_fence");

		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		ZE_DX_THROW_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&computeQueue)));
		ZE_DX_SET_ID(computeQueue, "compute_queue");
		ZE_DX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&computeFence)));
		ZE_DX_SET_ID(computeFence, "compute_fence");

		desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		ZE_DX_THROW_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&copyQueue)));
		ZE_DX_SET_ID(copyQueue, "copy_queue");
		ZE_DX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&copyFence)));
		ZE_DX_SET_ID(copyFence, "copy_fence");

		descriptorGpuAllocator.Init(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, descriptorCount, 1, 3, this);
		descriptorCpuAllocator.Init(D3D12_DESCRIPTOR_HEAP_FLAG_NONE, CPU_DESCRIPTOR_CHUNK_SIZE, 1, 3);
		descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Query feature support
		D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
		ZE_WIN_THROW_FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		ZE_WIN_THROW_FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
		D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16 = {};
		ZE_WIN_THROW_FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(options16)));

		// Check for RT
		switch (options5.RaytracingTier)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
			Settings::RayTracingTier = GFX::RayTracingTier::None;
			break;
		case D3D12_RAYTRACING_TIER_1_0:
			Settings::RayTracingTier = GFX::RayTracingTier::V1_0;
			break;
		case D3D12_RAYTRACING_TIER_1_1:
			Settings::RayTracingTier = GFX::RayTracingTier::V1_1;
			break;
		}

		allocator.Init(*this, options.ResourceHeapTier, options16.GPUUploadHeapSupported);
	}

	Device::~Device()
	{
		FreeXeSS();
		if (commandLists)
			commandLists.Free();

		switch (Settings::GpuVendor)
		{
		case GFX::VendorGPU::AMD:
		{
			agsDriverExtensionsDX12_DestroyDevice(gpuCtxAMD, device.Get(), nullptr);
			device.Detach();
			agsDeInitialize(gpuCtxAMD);
			break;
		}
		default:
			break;
		}
		descriptorGpuAllocator.DestroyFreeChunks(nullptr);
		descriptorCpuAllocator.DestroyFreeChunks(nullptr);

#if !_ZE_MODE_RELEASE
		if (pixCapturer)
		{
			const BOOL res = FreeLibrary(pixCapturer);
			ZE_ASSERT(res, "Error unloading WinPixGpuCapturer.dll!");
		}
#endif
		if (ffxApiDll)
		{
			const BOOL res = FreeLibrary(ffxApiDll);
			ZE_ASSERT(res, "Error unloading amd_fidelityfx_dx12.dll!");
			ffxApiDll = nullptr;
		}
	}

	const GFX::FfxApiFunctions* Device::GetFfxFunctions() noexcept
	{
		if (!ffxApiDll)
		{
			ffxApiDll = LoadLibraryW(L"amd_fidelityfx_dx12.dll");
			if (!ffxApiDll)
			{
				Logger::Error("Error loading [amd_fidelityfx_dx12.dll]!");
				return nullptr;
			}

			ffxCreateContext = (PfnFfxCreateContext)GetProcAddress(ffxApiDll, "ffxCreateContext");
			ffxFunctions.DestroyContext = (PfnFfxDestroyContext)GetProcAddress(ffxApiDll, "ffxDestroyContext");
			ffxFunctions.Configure = (PfnFfxConfigure)GetProcAddress(ffxApiDll, "ffxConfigure");
			ffxFunctions.Query = (PfnFfxQuery)GetProcAddress(ffxApiDll, "ffxQuery");
			ffxFunctions.Dispatch = (PfnFfxDispatch)GetProcAddress(ffxApiDll, "ffxDispatch");
		}
		return &ffxFunctions;
	}

	ffxReturnCode_t Device::CreateFfxCtx(ffxContext* ctx, ffxCreateContextDescHeader& ctxHeader) noexcept
	{
		if (GetFfxFunctions())
		{
			ffxCreateBackendDX12Desc backendDesc = { FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12, ctxHeader.pNext };
			backendDesc.device = device.Get();
			ctxHeader.pNext = &backendDesc.header;
			ffxReturnCode_t ret = ffxCreateContext(ctx, &ctxHeader, nullptr);
			ctxHeader.pNext = backendDesc.header.pNext;
			return ret;
		}
		return FFX_API_RETURN_ERROR_PARAMETER;
	}

	xess_context_handle_t Device::GetXeSSCtx()
	{
		if (xessData.Ctx == nullptr)
		{
			ZE_XESS_ENABLE();
			ZE_XESS_THROW_FAILED(xessD3D12CreateContext(device.Get(), &xessData.Ctx), "Error creating XeSS D3D12 context!");
			if (xessIsOptimalDriver(xessData.Ctx) == XESS_RESULT_WARNING_OLD_DRIVER)
				Logger::Warning("Outdated Intel driver!");
		}
		return xessData.Ctx;
	}

	void Device::InitializeXeSS(UInt2 targetRes, xess_quality_settings_t quality, U32 initFlags)
	{
		ZE_ASSERT(!IsXeSSEnabled(), "XeSS already initialized!");
		ZE_XESS_ENABLE();
		xess_context_handle_t ctx = GetXeSSCtx();

		xessData.TargetRes = { targetRes.X, targetRes.Y };
		xessData.Quality = quality;
		xessData.InitFlags = initFlags | XESS_INIT_FLAG_EXTERNAL_DESCRIPTOR_HEAP;
		ZE_XESS_THROW_FAILED(xessD3D12BuildPipelines(ctx, nullptr, false, initFlags), "Error creating XeSS D3D12 pipelines!");

		// Init external descriptor pool
		xess_properties_t props = {};
		ZE_XESS_THROW_FAILED(xessGetProperties(ctx, &xessData.TargetRes, &props), "Error querity XeSS properties!");
		xessData.Descs = AllocDescs(props.requiredDescriptorCount * Settings::GetBackbufferCount());
	}

	void Device::FreeXeSS() noexcept
	{
		if (xessData.Ctx)
		{
			ZE_XESS_ENABLE();
			ZE_XESS_CHECK(xessDestroyContext(xessData.Ctx), "Error destroying XeSS context!");
			xessData.Ctx = nullptr;
		}
		if (xessData.Descs.Handle)
			FreeDescs(xessData.Descs);

		xessData.TargetRes = { 0, 0 };
		xessData.InitFlags = 0;
		xessData.BufferRegion = INVALID_RID;
		xessData.TextureRegion = INVALID_RID;
	}

	std::pair<U64, U64> Device::GetXeSSAliasableRegionSizes() const
	{
		ZE_ASSERT(xessData.Ctx != nullptr, "XeSS not initialized!");
		ZE_XESS_ENABLE();

		xess_properties_t props = {};
		ZE_XESS_THROW_FAILED(xessGetProperties(xessData.Ctx, &xessData.TargetRes, &props), "Error querity XeSS properties!");

		return { props.tempBufferHeapSize, props.tempTextureHeapSize };
	}

	GFX::ShaderModel Device::GetMaxShaderModel() const noexcept
	{
		D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_HIGHEST_SHADER_MODEL };
		if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(D3D12_FEATURE_DATA_SHADER_MODEL))))
		{
			switch (shaderModel.HighestShaderModel)
			{
			case D3D_SHADER_MODEL_5_1:
				return GFX::ShaderModel::V5_1;
			case D3D_SHADER_MODEL_6_0:
				return GFX::ShaderModel::V6_0;
			case D3D_SHADER_MODEL_6_1:
				return GFX::ShaderModel::V6_1;
			case D3D_SHADER_MODEL_6_2:
				return GFX::ShaderModel::V6_2;
			case D3D_SHADER_MODEL_6_3:
				return GFX::ShaderModel::V6_3;
			case D3D_SHADER_MODEL_6_4:
				return GFX::ShaderModel::V6_4;
			case D3D_SHADER_MODEL_6_5:
				return GFX::ShaderModel::V6_5;
			case D3D_SHADER_MODEL_6_6:
				return GFX::ShaderModel::V6_6;
			case D3D_SHADER_MODEL_6_7:
				return GFX::ShaderModel::V6_7;
			case D3D_SHADER_MODEL_6_8:
				return GFX::ShaderModel::V6_8;
			default:
				ZE_WARNING("Shader model reported outside max known version 6.9, newer hardware detected!");
				[[fallthrough]];
			case D3D_SHADER_MODEL_6_9:
				return GFX::ShaderModel::V6_9;
			}
		}
		return GFX::ShaderModel::V6_0;
	}

	std::pair<U32, U32> Device::GetWaveLaneCountRange() const noexcept
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1 = {};
		if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &options1, sizeof(options1))))
			return { options1.WaveLaneCountMin, options1.WaveLaneCountMax };
		// Minimal known wave size is 32
		return { 32, 32 };
	}

	bool Device::IsShaderFloat16Supported() const noexcept
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS4 options4 = {};
		if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &options4, sizeof(options4))))
			return options4.Native16BitShaderOpsSupported;
		return false;
	}

	void Device::Execute(GFX::CommandList* cls, U32 count)
	{
		if (count == 1)
		{
			switch (cls->Get().dx12.GetList()->GetType())
			{
			default:
				ZE_FAIL("Incorrect type of command list!!!"); [[fallthrough]];
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
				return ExecuteMain(*cls);
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
				return ExecuteCompute(*cls);
			case D3D12_COMMAND_LIST_TYPE_COPY:
				return ExecuteCopy(*cls);
			}
		}

		// Find max size for command lists to execute at once
		U32 mainCount = 0, computeCount = 0, copyCount = 0;
		for (U32 i = 0; i < count; ++i)
		{
			ZE_ASSERT(cls[i].Get().dx12.GetList() != nullptr, "Empty command list!");

			switch (cls[i].Get().dx12.GetList()->GetType())
			{
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
			{
				++mainCount;
				break;
			}
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			{
				++computeCount;
				break;
			}
			case D3D12_COMMAND_LIST_TYPE_COPY:
			{
				++copyCount;
				break;
			}
			default:
				ZE_FAIL("Incorrect type of command list!!!");
			}
		}

		// Realloc if needed bigger list
		count = std::max(commandListsCount, mainCount);
		if (computeCount > count)
			count = computeCount;
		if (copyCount > count)
			count = copyCount;
		if (count > commandListsCount)
		{
			commandListsCount = count;
			commandLists = reinterpret_cast<ICommandList**>(realloc(commandLists, count * sizeof(ICommandList*)));
		}

		// Execute lists
		if (mainCount)
		{
			U32 i = 0, j = 0;
			do
			{
				if (cls[i].Get().dx12.GetList()->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT)
					commandLists[i++] = cls[j].Get().dx12.GetList();
				++j;
			} while (i < mainCount);
			ZE_DX_THROW_FAILED_INFO(mainQueue->ExecuteCommandLists(mainCount, commandLists));
		}
		if (computeCount)
		{
			U32 i = 0, j = 0;
			do
			{
				if (cls[i].Get().dx12.GetList()->GetType() == D3D12_COMMAND_LIST_TYPE_COMPUTE)
					commandLists[i++] = cls[j].Get().dx12.GetList();
				++j;
			} while (i < computeCount);
			ZE_DX_THROW_FAILED_INFO(computeQueue->ExecuteCommandLists(computeCount, commandLists));
		}
		if (copyCount)
		{
			U32 i = 0, j = 0;
			do
			{
				if (cls[i].Get().dx12.GetList()->GetType() == D3D12_COMMAND_LIST_TYPE_COPY)
					commandLists[i++] = cls[j].Get().dx12.GetList();
				++j;
			} while (i < copyCount);
			ZE_DX_THROW_FAILED_INFO(copyQueue->ExecuteCommandLists(copyCount, commandLists));
		}
	}

	void Device::ExecuteMain(GFX::CommandList& cl)
	{
		Execute(mainQueue.Get(), cl.Get().dx12);
	}

	void Device::ExecuteCompute(GFX::CommandList& cl)
	{
		Execute(computeQueue.Get(), cl.Get().dx12);
	}

	void Device::ExecuteCopy(GFX::CommandList& cl)
	{
		Execute(copyQueue.Get(), cl.Get().dx12);
	}

	D3D12_RESOURCE_DESC1 Device::GetBufferDesc(U64 size) const noexcept
	{
		ZE_ASSERT(size, "Cannot create empty buffer!");

		D3D12_RESOURCE_DESC1 desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Width = size;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		desc.SamplerFeedbackMipRegion.Width = 0;
		desc.SamplerFeedbackMipRegion.Height = 0;
		desc.SamplerFeedbackMipRegion.Depth = 0;
		return desc;
	}

	D3D12_RESOURCE_DESC1 Device::GetTextureDesc(U32 width, U32 height, U16 count,
		DXGI_FORMAT format, GFX::Resource::Texture::Type type) const noexcept
	{
		ZE_ASSERT(width < D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION
			&& height < D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
			"Texture too big!");
		ZE_ASSERT(height == 1 || type != GFX::Resource::Texture::Type::Tex1D, "Height of 1D texture must be 1!");

		D3D12_RESOURCE_DESC1 desc = {};
		desc.Dimension = GetTextureDimension(type);
		desc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Width = width;
		desc.Height = height;
		desc.DepthOrArraySize = count;
		desc.MipLevels = 1;
		desc.Format = format; // Maybe not all formats supported on given hardware, if strange formats to be used check D3D12_FORMAT_SUPPORT1
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		desc.SamplerFeedbackMipRegion.Width = 0;
		desc.SamplerFeedbackMipRegion.Height = 0;
		desc.SamplerFeedbackMipRegion.Depth = 0;

		D3D12_RESOURCE_ALLOCATION_INFO1 info = {};
		device->GetResourceAllocationInfo2(0, 1, &desc, &info);
		if (info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
			desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

		return desc;
	}

	ResourceInfo Device::CreateBuffer(const D3D12_RESOURCE_DESC1& desc, bool dynamic)
	{
		if (dynamic)
			return allocator.AllocDynamicBuffer(*this, desc);
		return allocator.AllocBuffer(*this, desc);
	}

	ResourceInfo Device::CreateTexture(const D3D12_RESOURCE_DESC1& desc)
	{
		D3D12_RESOURCE_ALLOCATION_INFO1 info = {};
		device->GetResourceAllocationInfo2(0, 1, &desc, &info);

		if (desc.Alignment == D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
			return allocator.AllocTexture_64KB(*this, info.SizeInBytes, desc);
		else if (desc.Alignment == D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
			return allocator.AllocTexture_4KB(*this, info.SizeInBytes, desc);
		return allocator.AllocTexture_4MB(*this, info.SizeInBytes, desc);
	}

	DescriptorInfo Device::AllocDescs(U32 count, bool gpuHeap) noexcept
	{
		ZE_ASSERT(count > 0, "Cannot allocate empty descriptors!");

		DescriptorAllocator& descAlloc = gpuHeap ? descriptorGpuAllocator : descriptorCpuAllocator;

		DescriptorInfo rangeStart = {};
		rangeStart.Handle = descAlloc.Alloc(count, 1, this);
		rangeStart.GpuSide = gpuHeap;

		if (rangeStart.Handle != nullptr)
		{
			const U64 offset = descAlloc.GetOffset(rangeStart.Handle) * descriptorSize;
			rangeStart.GPU.ptr = gpuHeap ? descAlloc.GetMemory(nullptr).Heap->GetGPUDescriptorHandleForHeapStart().ptr + offset : 0;
			rangeStart.CPU.ptr = descAlloc.GetMemory(rangeStart.Handle).Heap->GetCPUDescriptorHandleForHeapStart().ptr + offset;
		}
		else
		{
			ZE_FAIL("Run out of descriptors, make sure to configure engine with correct number of descriptors at the start!");
		}
		return rangeStart;
	}

	void Device::FreeDescs(DescriptorInfo& descInfo) noexcept
	{
		(descInfo.GpuSide ? descriptorGpuAllocator : descriptorCpuAllocator).Free(descInfo.Handle, this);

		descInfo.Handle = nullptr;
		descInfo.GPU.ptr = 0;
		descInfo.CPU.ptr = 0;
	}

	U32 Device::GetXeSSDescriptorsOffset() const noexcept
	{
		ZE_ASSERT(IsXeSSEnabled(), "XeSS hasn't been initializd yet!");
		U64 heapOffset = xessData.Descs.GPU.ptr - GetDescHeap()->GetGPUDescriptorHandleForHeapStart().ptr;
		U64 singleSetCount = descriptorGpuAllocator.GetSize(xessData.Descs.Handle) / Settings::GetBackbufferCount();
		return Utils::SafeCast<U32>(heapOffset + descriptorSize * singleSetCount * Settings::GetCurrentBackbufferIndex());
	}
}