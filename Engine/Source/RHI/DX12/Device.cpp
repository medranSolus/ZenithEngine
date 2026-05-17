#include "RHI/DX12/Device.h"
#include "RHI/DX12/DREDRecovery.h"
#include "RHI/DX12/GarbageCollector.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::DX12
{
	Status Device::DescHeap::Init(DescHeap& chunk, Allocator::TLSFMemoryChunkFlags flags, U64 size, void* userData) noexcept
	{
		ZE_ASSERT(userData, "Empty device data!");
		Device& dev = *reinterpret_cast<Device*>(userData);

		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
		descHeapDesc.NodeMask = 0;
		descHeapDesc.Flags = static_cast<D3D12_DESCRIPTOR_HEAP_FLAGS>(flags);
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descHeapDesc.NumDescriptors = Utils::SafeCast<U32>(size);
		ZE_DX_RET_FAILED(dev.GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&chunk.Heap)));
		return {};
	}

	Status Device::WaitCPU(IFence* fence, U64 val) noexcept
	{
		if (fence->GetCompletedValue() < val)
		{
			HANDLE fenceEvent = CreateEventW(nullptr, false, false, nullptr);
			ZE_WIN_RET_FAILED_LAST(fenceEvent == nullptr);

			ZE_DX_RET_FAILED(fence->SetEventOnCompletion(val, fenceEvent));
			ZE_WIN_RET_FAILED_LAST(WaitForSingleObject(fenceEvent, INFINITE) != WAIT_OBJECT_0);
			ZE_WIN_RET_FAILED_LAST(CloseHandle(fenceEvent) == 0);
		}
		return {};
	}

	Status Device::WaitGPU(IFence* fence, ICommandQueue* queue, U64 val) noexcept
	{
		ZE_DX_RET_FAILED(queue->Wait(fence, val));
		return {};
	}

	Expected<U64> Device::SetFenceCPU(IFence* fence, UA64& fenceVal) noexcept
	{
		U64 val = ++fenceVal;
		ZE_DX_RET_FAILED_EXPECT(fence->Signal(val));
		return val;
	}

	Expected<U64> Device::SetFenceGPU(IFence* fence, ICommandQueue* queue, UA64& fenceVal) noexcept
	{
		U64 val = ++fenceVal;
		ZE_DX_RET_FAILED_EXPECT(queue->Signal(fence, val));
		return val;
	}

	void Device::Execute(ICommandQueue* queue, CommandList& cl) noexcept
	{
		ZE_ASSERT(cl.GetList() != nullptr, "Empty list!");
		ICommandList* lists[] = { cl.GetList() };
		ZE_DX_CHECK_FAILED(queue->ExecuteCommandLists(1, lists), "ExecuteCommandLists caused debug layer messages!");
	}

	void Device::MoveFrom(Device& dev) noexcept
	{
#if _ZE_DEBUG_GFX_API
		debugManager = std::move(dev.debugManager);
		DX::DebugInfoManager::Register(debugManager);
#endif
		device = std::move(dev.device);
		mainQueue = std::move(dev.mainQueue);
		computeQueue = std::move(dev.computeQueue);
		copyQueue = std::move(dev.copyQueue);

		mainFenceVal = dev.mainFenceVal.load();
		mainFence = std::move(dev.mainFence);
		computeFenceVal = dev.computeFenceVal.load();
		computeFence = std::move(dev.computeFence);
		copyFenceVal = dev.copyFenceVal.load();
		copyFence = std::move(dev.copyFence);

		allocator = std::move(dev.allocator);

		descriptorSize = dev.descriptorSize;
		gpuCtx = std::exchange(dev.gpuCtx, {});

#if !_ZE_MODE_RELEASE
		pixCapturer = std::exchange(dev.pixCapturer, nullptr);
#endif
		featureExistingHeap = dev.featureExistingHeap;
		displayProps = std::move(dev.displayProps);
	}

	Device::Device() noexcept
		: blockDescAllocator(std::make_shared<DescriptorAllocator::BlockAllocator>(BLOCK_DESCRIPTOR_ALLOC_CAPACITY)),
		chunkDescAllocator(std::make_shared<DescriptorAllocator::ChunkAllocator>(CHUNK_DESCRIPTOR_ALLOC_CAPACITY)),
		descriptorGpuAllocator(blockDescAllocator, chunkDescAllocator, true),
		descriptorCpuAllocator(blockDescAllocator, chunkDescAllocator)
	{}

	Device::Device(Device&& dev) noexcept
		: blockDescAllocator(std::move(dev.blockDescAllocator)), chunkDescAllocator(std::move(dev.chunkDescAllocator)),
		descriptorGpuAllocator(std::move(dev.descriptorGpuAllocator)),
		descriptorCpuAllocator(std::move(dev.descriptorCpuAllocator))
	{
		MoveFrom(dev);
	}

	Device& Device::operator=(Device&& dev) noexcept
	{
		blockDescAllocator = std::move(dev.blockDescAllocator);
		chunkDescAllocator = std::move(dev.chunkDescAllocator);
		descriptorGpuAllocator = std::move(dev.descriptorGpuAllocator);
		descriptorCpuAllocator = std::move(dev.descriptorCpuAllocator);
		MoveFrom(dev);
		return *this;
	}

	Device::~Device()
	{
		if (device)
		{
			GarbageCollector::Get().Flush(*this);
			switch (Settings::GpuVendor)
			{
			case GFX::VendorGPU::AMD:
			{
				agsDriverExtensionsDX12_DestroyDevice(gpuCtx.AMD, device.Get(), nullptr);
				device.Detach();
				agsDeInitialize(gpuCtx.AMD);
				break;
			}
			default:
				break;
			}
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
	}

	Expected<Device> Device::Create(const Window::MainWindow& window, U32 descriptorCount) noexcept
	{
		Device dev = {};
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
				dev.pixCapturer = LoadLibraryW((pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll").c_str());
				if (dev.pixCapturer == nullptr)
				{
					ZE_CODE_WARNING(ZE_WIN_LAST_ERROR(), "Failed to load \"WinPixGpuCapturer.dll\"");
				}
			}
		}
#endif

#if _ZE_DEBUG_GFX_API
		// Enable Debug Layer before calling any DirectX commands
		DX::ComPtr<IDebug> debugInterface = nullptr;
		ZE_DX_RET_FAILED_EXPECT(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();
		if (Settings::IsEnabledGPUValidation())
			debugInterface->SetEnableGPUBasedValidation(true);

		ZE_EXPECT_RET_FAILED(dev.debugManager, DX::DebugInfoManager::Create());
		DX::DebugInfoManager::Register(dev.debugManager);

		// Enable device removed recovery
		DREDRecovery::Enable();
#endif

		// Find adapter
		DX::ComPtr<DX::IFactory> factory;
		ZE_EXPECT_RET_FAILED(factory, DX::CreateFactory());
		DX::ComPtr<DX::IAdapter> adapter = DX::CreateAdapter(factory);
		if (adapter == nullptr)
		{
			ZE_DX_RET_FAILED_EXPECT(DX::Error::NO_ADAPTER_ERROR);
		}

		auto initDevice = [&](D3D_FEATURE_LEVEL level) noexcept -> HRESULT
			{
				HRESULT hr = E_FAIL;
				// Initialize via hardware specific functions
				switch (Settings::GpuVendor)
				{
				case GFX::VendorGPU::AMD:
				{
					AGSConfiguration agsConfig = {};
					if (agsInitialize(AGS_MAKE_VERSION(AMD_AGS_VERSION_MAJOR, AMD_AGS_VERSION_MINOR, AMD_AGS_VERSION_PATCH),
						&agsConfig, &dev.gpuCtx.AMD, nullptr) == AGS_SUCCESS)
					{
						AGSDX12DeviceCreationParams deviceParams = {};
						deviceParams.pAdapter = adapter.Get();
						deviceParams.iid = __uuidof(device);
						deviceParams.FeatureLevel = MINIMAL_D3D_LEVEL;

						AGSDX12ExtensionParams extensionParams = {};
						AGSDX12ReturnedParams returnParams;
						if (agsDriverExtensionsDX12_CreateDevice(dev.gpuCtx.AMD, &deviceParams, &extensionParams, &returnParams) == AGS_SUCCESS)
						{
							hr = returnParams.pDevice->QueryInterface(IID_PPV_ARGS(&dev.device));
							returnParams.pDevice->Release();
							break;
						}
						agsDeInitialize(dev.gpuCtx.AMD);
						dev.gpuCtx.AMD = nullptr;
					}
					Settings::GpuVendor = GFX::VendorGPU::Unknown;
					break;
				}
				default:
					break;
				}

				// Failed to create GPU specific device
				if (dev.device == nullptr)
					hr = D3D12CreateDevice(adapter.Get(), MINIMAL_D3D_LEVEL, IID_PPV_ARGS(&dev.device));
				return hr;
			};

		std::array<D3D_FEATURE_LEVEL, 4> featureLevels =
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1
		};
		HRESULT hr = E_FAIL;
		for (D3D_FEATURE_LEVEL level : featureLevels)
		{
			hr = initDevice(level);
			if (SUCCEEDED(hr))
			{
				Logger::Info("D3D12 device created with feature level " + std::to_string(level >> 12) + "_" + std::to_string((level >> 8) & 0xF));
				break;
			}
		}
		ZE_DX_RET_FAILED_NO_DEBUG_EXPECT(hr);
		ZE_DX_CHECK_DEBUG_INFO(hr); // Suppress any debug info generated during device creation, ex. for GBV

		D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = {};
		ZE_DX_RET_FAILED_EXPECT(dev.device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12)));
		if (!options12.EnhancedBarriersSupported)
		{
			ZE_DX_RET_FAILED_EXPECT(DX::Error::ENHANCED_BARRIERS_ERROR);
		}

#if _ZE_DEBUG_GFX_API
		DX::ComPtr<IInfoQueue> infoQueue = nullptr;
		hr = dev.device.As(&infoQueue);
		if (SUCCEEDED(hr))
		{
			// Set breaks on dangerous messages
			hr = infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			if (FAILED(hr))
			{
				ZE_CODE_WARNING(DX::Error::Make(hr), "Failed to set breakpoints on corruption D3D12 messages!");
			}
			hr = infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			if (FAILED(hr))
			{
				ZE_CODE_WARNING(DX::Error::Make(hr), "Failed to set breakpoints on error D3D12 messages!");
			}

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
				D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED,
				// Bug in AgilitySDK 1.615.1 in conjuction with enhanced barriers where even when creating heap without not-zeroed flag it reports same issue. TODO: remove when fixed
				D3D12_MESSAGE_ID_RENDER_TARGET_OR_DEPTH_STENCIL_RESOUCE_NOT_INITIALIZED,
			};

			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumSeverities = 1;
			filter.DenyList.pSeverityList = severities;
			filter.DenyList.NumIDs = sizeof(denyIds) / sizeof(D3D12_MESSAGE_ID);
			filter.DenyList.pIDList = denyIds;

			hr = infoQueue->PushStorageFilter(&filter);
			if (FAILED(hr))
			{
				ZE_CODE_WARNING(DX::Error::Make(hr), "Failed to set up filters on D3D12 messages!");
			}
		}
		else
		{
			ZE_CODE_WARNING(DX::Error::Make(hr), "Failed to access InfoQueue interface, falling back to default debug layer behavior!");
		}

		if (Settings::IsEnabledGPUValidation())
		{
			DX::ComPtr<IDebugDevice> debugDevice = nullptr;
			hr = dev.device.As(&debugDevice);
			if (SUCCEEDED(hr))
			{
				const D3D12_DEBUG_FEATURE debugFeature = D3D12_DEBUG_FEATURE_ALLOW_BEHAVIOR_CHANGING_DEBUG_AIDS;
				hr = debugDevice->SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_FEATURE_FLAGS,
					&debugFeature, sizeof(D3D12_DEBUG_FEATURE));
				if (FAILED(hr))
				{
					ZE_CODE_WARNING(DX::Error::Make(hr), "Failed to set debug device parameter feature flags!");
				}

				D3D12_DEBUG_DEVICE_GPU_BASED_VALIDATION_SETTINGS validationSettings = {};
				// Should cover all messages
				validationSettings.MaxMessagesPerCommandList = 1024;
				// Can avoid most cases of TDRs
				validationSettings.DefaultShaderPatchMode = D3D12_GPU_BASED_VALIDATION_SHADER_PATCH_MODE_GUARDED_VALIDATION;
				validationSettings.PipelineStateCreateFlags = D3D12_GPU_BASED_VALIDATION_PIPELINE_STATE_CREATE_FLAG_FRONT_LOAD_CREATE_GUARDED_VALIDATION_SHADERS;
				hr = debugDevice->SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_GPU_BASED_VALIDATION_SETTINGS,
					&validationSettings, sizeof(D3D12_DEBUG_DEVICE_GPU_BASED_VALIDATION_SETTINGS));
				if (FAILED(hr))
				{
					ZE_CODE_WARNING(DX::Error::Make(hr), "Failed to set GPU based validation settings!");
				}
			}
			else
			{
				ZE_CODE_WARNING(DX::Error::Make(hr), "Failed to access DebugDevice interface, requested GPU based validation impacted!");
			}
		}
#endif
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ZE_DX_RET_FAILED_EXPECT(dev.device->CreateCommandQueue(&desc, IID_PPV_ARGS(&dev.mainQueue)));
		ZE_DX_SET_ID(dev.mainQueue, "direct_queue");
		ZE_DX_RET_FAILED_EXPECT(dev.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dev.mainFence)));
		ZE_DX_SET_ID(dev.mainFence, "direct_fence");

		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		ZE_DX_RET_FAILED_EXPECT(dev.device->CreateCommandQueue(&desc, IID_PPV_ARGS(&dev.computeQueue)));
		ZE_DX_SET_ID(dev.computeQueue, "compute_queue");
		ZE_DX_RET_FAILED_EXPECT(dev.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dev.computeFence)));
		ZE_DX_SET_ID(dev.computeFence, "compute_fence");

		desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		ZE_DX_RET_FAILED_EXPECT(dev.device->CreateCommandQueue(&desc, IID_PPV_ARGS(&dev.copyQueue)));
		ZE_DX_SET_ID(dev.copyQueue, "copy_queue");
		ZE_DX_RET_FAILED_EXPECT(dev.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dev.copyFence)));
		ZE_DX_SET_ID(dev.copyFence, "copy_fence");

		if (Status code = dev.descriptorGpuAllocator.Init(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, descriptorCount, 1, 3, &dev))
		{
			Logger::Error("Failed to initialize DX12 GPU descriptor heap allocator!");
			return std::unexpected(code);
		}
		if (Status code = dev.descriptorCpuAllocator.Init(D3D12_DESCRIPTOR_HEAP_FLAG_NONE, CPU_DESCRIPTOR_CHUNK_SIZE, 1, 3))
		{
			Logger::Error("Failed to initialize DX12 CPU descriptor heap allocator!");
			return std::unexpected(code);
		}
		dev.descriptorSize = dev.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Query feature support
		D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
		if (FAILED(dev.device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
			options.ResourceHeapTier = D3D12_RESOURCE_HEAP_TIER_1;

		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		if (FAILED(dev.device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
			options5.RaytracingTier = D3D12_RAYTRACING_TIER_NOT_SUPPORTED;

		D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16 = {};
		if (FAILED(dev.device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(options16))))
			options16.GPUUploadHeapSupported = false;

		D3D12_FEATURE_DATA_EXISTING_HEAPS existingHeaps = {};
		if (SUCCEEDED(dev.device->CheckFeatureSupport(D3D12_FEATURE_EXISTING_HEAPS, &existingHeaps, sizeof(existingHeaps))))
			dev.featureExistingHeap = existingHeaps.Supported;

		D3D12_FEATURE_DATA_TIGHT_ALIGNMENT tightAlignment = {};
		if (FAILED(dev.device->CheckFeatureSupport(D3D12_FEATURE_D3D12_TIGHT_ALIGNMENT, &tightAlignment, sizeof(tightAlignment))))
			tightAlignment.SupportTier = D3D12_TIGHT_ALIGNMENT_TIER_NOT_SUPPORTED;
		// TODO: Enable when DirectStorage stop complaining about size of the destination resource and will be able to perform normal copies
		tightAlignment.SupportTier = D3D12_TIGHT_ALIGNMENT_TIER_NOT_SUPPORTED;

		ZE_EXPECT_RET_FAILED(dev.allocator, AllocatorGPU::Create(dev, options.ResourceHeapTier, options16.GPUUploadHeapSupported, tightAlignment.SupportTier));

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
		case D3D12_RAYTRACING_TIER_1_2:
			Settings::RayTracingTier = GFX::RayTracingTier::V1_2;
			break;
		}

		// No support for 8 bit indices on DirectX
		Settings::SetU8IndexBuffers(false);
		Settings::SetGfxSupportSSSR(true);

		return dev;
	}

	void Device::OnMonitorChanged(const Window::MainWindow& window) noexcept
	{
		bool found = false;
		auto exp = DX::CreateFactory();
		if (exp)
		{
			DX::ComPtr<DX::IFactory> factory = std::move(exp.value());
			// Enumerate available outputs and find the one attached to our window
			HMONITOR monitor = MonitorFromWindow(window.GetHandle(), MONITOR_DEFAULTTONEAREST);
			for (U32 i = 0; !found; ++i)
			{
				// Need to iterate over whole list of adapters again in case that current GPU doesn't own the output (in case of systems with integrated graphics)
				DX::ComPtr<DX::IAdapter> tempAdapter = nullptr;
				if (SUCCEEDED(factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tempAdapter))))
				{
					for (U32 j = 0; true; ++j)
					{
						DX::ComPtr<IDXGIOutput> tempOutput = nullptr;
						if (SUCCEEDED(tempAdapter->EnumOutputs(j, &tempOutput)))
						{
							DX::ComPtr<DX::IOutput> output = nullptr;
							if (SUCCEEDED(tempOutput.As(&output)))
							{
								DXGI_OUTPUT_DESC1 desc;
								if (SUCCEEDED(output->GetDesc1(&desc)))
								{
									if (monitor == desc.Monitor)
									{
										displayProps.RedPrimary = { desc.RedPrimary[0], desc.RedPrimary[1] };
										displayProps.GreenPrimary = { desc.GreenPrimary[0], desc.GreenPrimary[1] };
										displayProps.BluePrimary = { desc.BluePrimary[0], desc.BluePrimary[1] };
										displayProps.WhitePoint = { desc.WhitePoint[0], desc.WhitePoint[1] };
										displayProps.MinLuminance = desc.MinLuminance;
										displayProps.MaxLuminance = desc.MaxLuminance;
										found = true;
										break;
									}
								}
							}
						}
						else
							break;
					}
				}
				else
					break;
			}
			if (!found)
				Logger::Warning("DX12 warning: Cannot find monitor attached to main window, using default display properties!");
		}
		else
		{
			ZE_CODE_WARNING(exp.error(), "Cannot create DXGI factory to query monitor infomation, using default display properties!");
		}
		if (!found)
		{
			// Default CIE 1931 xy chromaticity values for sRGB / Rec.709 
			displayProps.RedPrimary = { 0.64f, 0.33f };
			displayProps.GreenPrimary = { 0.3f, 0.6f };
			displayProps.BluePrimary = { 0.15f, 0.06f };
			displayProps.WhitePoint = { 0.3127f, 0.329f };
			displayProps.MinLuminance = 0.0f;
			displayProps.MaxLuminance = 300.0f;
		}
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

	void Device::Execute(GFX::CommandList* cls, U32 count) const noexcept
	{
		ZE_ASSERT(count > 0 && cls, "No valid command lists provided!");
		if (count == 1)
		{
			switch (cls->Get().dx12.GetList()->GetType())
			{
			default:
				ZE_ENUM_UNHANDLED();
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
			default:
				ZE_ENUM_UNHANDLED();
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
			}
		}

		// Execute lists
		std::vector<ICommandList*> commandLists(std::max(mainCount, std::max(computeCount, copyCount)));
		if (mainCount)
		{
			U32 i = 0, j = 0;
			do
			{
				if (cls[i].Get().dx12.GetList()->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT)
					commandLists.at(i++) = cls[j].Get().dx12.GetList();
				++j;
			} while (i < mainCount);
			ZE_DX_CHECK_FAILED(mainQueue->ExecuteCommandLists(mainCount, commandLists.data()), "Executing DIRECT command lists produced debug layer messages!");
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
			ZE_DX_CHECK_FAILED(computeQueue->ExecuteCommandLists(computeCount, commandLists.data()), "Executing COMPUTE command lists produced debug layer messages!");
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
			ZE_DX_CHECK_FAILED(copyQueue->ExecuteCommandLists(copyCount, commandLists.data()), "Executing COPY command lists produced debug layer messages!");
		}
	}

	void Device::ExecuteMain(GFX::CommandList& cl) const noexcept
	{
		Execute(mainQueue.Get(), cl.Get().dx12);
	}

	void Device::ExecuteCompute(GFX::CommandList& cl) const noexcept
	{
		Execute(computeQueue.Get(), cl.Get().dx12);
	}

	void Device::ExecuteCopy(GFX::CommandList& cl) const noexcept
	{
		Execute(copyQueue.Get(), cl.Get().dx12);
	}

	Expected<FfxBreadcrumbsBlockData> Device::AllocBreadcrumbsBlock(U64 bytes) noexcept
	{
		FfxBreadcrumbsBlockData blockData = {};
		D3D12_RESOURCE_DESC1 desc = GetBufferDesc(bytes);
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;

		// VirtualAlloc() + OpenExistingHeapFromAddress() + CreatePlacedResource() path, ensures that Breadcrumb buffer survives TDR.
		if (featureExistingHeap)
		{
			blockData.memory = VirtualAlloc(nullptr, bytes, MEM_COMMIT, PAGE_READWRITE);
			if (blockData.memory != nullptr)
			{
				IHeap* heap = nullptr;
				if (SUCCEEDED(device->OpenExistingHeapFromAddress(blockData.memory, IID_PPV_ARGS(&heap))))
				{
					IResource* resource = nullptr;
					if (SUCCEEDED(device->CreatePlacedResource2(heap, 0, &desc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, IID_PPV_ARGS(&resource))))
					{
						resource->SetName(L"Buffer for Breadcrumbs - placed in VirtualAlloc, OpenExistingHeapFromAddress");
						blockData.heap = reinterpret_cast<void*>(heap);
						blockData.buffer = reinterpret_cast<void*>(resource);
						blockData.baseAddress = resource->GetGPUVirtualAddress();
						return blockData;
					}
					heap->Release();
				}
				[[maybe_unused]] const BOOL status = VirtualFree(blockData.memory, 0, MEM_RELEASE);
				ZE_ASSERT(status != 0, "Error while releasing Breadcrumb memory!");
				blockData.memory = nullptr;
			}
		}

		// If VirtualAlloc path failed, try standard path
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		ResourceInfo res = {};
		ZE_EXPECT_RET_FAILED(res, allocator.AllocReadbackBuffer(*this, desc));

		const D3D12_RANGE range = {};
		ZE_DX_RET_FAILED_EXPECT(res.Resource->Map(0, &range, &blockData.memory));
		res.Resource->AddRef(); // Allow for release later on

		blockData.heap = res.Handle;
		blockData.buffer = reinterpret_cast<void*>(res.Resource.Get());
		blockData.baseAddress = res.Resource->GetGPUVirtualAddress();
		return blockData;
	}

	void Device::FreeBreadcrumbsBlock(FfxBreadcrumbsBlockData& block) noexcept
	{
		if (featureExistingHeap)
		{
			// VirutalAlloc() path
			if (block.buffer)
			{
				reinterpret_cast<IResource*>(block.buffer)->Release();
				block.buffer = nullptr;
			}
			if (block.heap)
			{
				reinterpret_cast<IHeap*>(block.heap)->Release();
				block.heap = nullptr;
			}
			if (block.memory)
			{
				[[maybe_unused]] const BOOL status = VirtualFree(block.memory, 0, MEM_RELEASE);
				ZE_ASSERT(status != 0, "Error while releasing Breadcrumb memory!");
				block.memory = nullptr;
			}
		}
		else if (block.buffer && block.heap)
		{
			ResourceInfo res = {};
			res.Handle = block.heap;
			res.Resource.Attach(reinterpret_cast<IResource*>(block.buffer));
			if (block.memory)
			{
				res.Resource->Unmap(0, nullptr);
				block.memory = nullptr;
			}

			allocator.RemoveReadbackBuffer(res);
			block.buffer = nullptr;
			block.heap = nullptr;
		}
	}

	void Device::EndFrame() noexcept
	{
		GarbageCollector::Get().AdvanceFrame(*this);
	}

	D3D12_RESOURCE_DESC1 Device::GetBufferDesc(U64 size) const noexcept
	{
		ZE_ASSERT(size, "Cannot create empty buffer!");

		D3D12_RESOURCE_DESC1 desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = allocator.IsTightAlignmentEnabled() ? 0 : D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Width = size;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = allocator.IsTightAlignmentEnabled() ? D3D12_RESOURCE_FLAG_USE_TIGHT_ALIGNMENT : D3D12_RESOURCE_FLAG_NONE;
		desc.SamplerFeedbackMipRegion.Width = 0;
		desc.SamplerFeedbackMipRegion.Height = 0;
		desc.SamplerFeedbackMipRegion.Depth = 0;
		return desc;
	}

	D3D12_RESOURCE_DESC1 Device::GetTextureDesc(U32 width, U32 height, U16 count,
		DXGI_FORMAT format, GFX::Resource::Texture::Type type) const noexcept
	{
		ZE_ASSERT(width < D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION && height < D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION, "Texture too big!");
		ZE_ASSERT(height == 1 || type != GFX::Resource::Texture::Type::Tex1D, "Height of 1D texture must be 1!");

		D3D12_RESOURCE_DESC1 desc = {};
		desc.Dimension = GetTextureDimension(type);
		desc.Alignment = allocator.IsTightAlignmentEnabled() ? 0 : D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Width = width;
		desc.Height = height;
		desc.DepthOrArraySize = count;
		desc.MipLevels = 1;
		desc.Format = format; // Maybe not all formats supported on given hardware, if strange formats to be used check D3D12_FORMAT_SUPPORT1
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = allocator.IsTightAlignmentEnabled() ? D3D12_RESOURCE_FLAG_USE_TIGHT_ALIGNMENT : D3D12_RESOURCE_FLAG_NONE;
		desc.SamplerFeedbackMipRegion.Width = 0;
		desc.SamplerFeedbackMipRegion.Height = 0;
		desc.SamplerFeedbackMipRegion.Depth = 0;

		if (!allocator.IsTightAlignmentEnabled())
		{
			D3D12_RESOURCE_ALLOCATION_INFO1 info = {};
			device->GetResourceAllocationInfo3(0, 1, &desc, nullptr, nullptr, &info);
			if (info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
				desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		}
		return desc;
	}

	Expected<ResourceInfo> Device::CreateBuffer(const D3D12_RESOURCE_DESC1& desc, bool dynamic) noexcept
	{
		if (dynamic)
			return allocator.AllocDynamicBuffer(*this, desc);
		return allocator.AllocBuffer(*this, desc);
	}

	Expected<ResourceInfo> Device::CreateTexture(const D3D12_RESOURCE_DESC1& desc) noexcept
	{
		return allocator.AllocTexture(*this, desc);
	}

	Expected<DescriptorInfo> Device::AllocDescs(U32 count, bool gpuHeap) noexcept
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
			return std::unexpected(DX::Error::Make(DX::Error::ALLOC_ERROR));
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
}