#include "RHI/VK/Device.h"
#include "RHI/VK/VulkanException.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::VK
{
	U16 Device::GetExtensionIndexDynamic(const char* extName) noexcept
	{
		// __COUNTER__ is supported on MSVC, GCC and Clang, on other compilers should find alternative
		constexpr U16 COUNTER_BASE = __COUNTER__ + 1;

#define X(ext) if (std::strcmp(ext, extName) == 0) return __COUNTER__ - COUNTER_BASE; else
		ZE_VK_EXT_LIST
#undef X
			return KNOWN_EXTENSION_COUNT;
	}

	void Device::WaitCPU(VkSemaphore fence, U64 val)
	{
		ZE_VK_ENABLE();

		VkSemaphoreWaitInfo waitInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO, nullptr };
		waitInfo.flags = 0;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &fence;
		waitInfo.pValues = &val;
		do
		{
			ZE_VK_THROW_FAILED(vkWaitSemaphores(device, &waitInfo, UINT64_MAX));
		} while (ZE_VK_EXCEPT_RESULT == VK_TIMEOUT);
	}

	void Device::WaitGPU(VkSemaphore fence, VkQueue queue, U64 val)
	{
		// TODO: expose stageMask with own enum (no effect on D3D12)
		ZE_VK_ENABLE();

		VkSemaphoreSubmitInfo waitInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr };
		waitInfo.semaphore = fence;
		waitInfo.value = val;
		waitInfo.stageMask = VK_PIPELINE_STAGE_2_NONE; // Second synch scope, wait completed before these commands
		waitInfo.deviceIndex = 0;

		VkSubmitInfo2 submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO_2, nullptr };
		submitInfo.flags = 0;
		submitInfo.waitSemaphoreInfoCount = 1;
		submitInfo.pWaitSemaphoreInfos = &waitInfo;
		submitInfo.commandBufferInfoCount = 0;
		submitInfo.pCommandBufferInfos = nullptr;
		submitInfo.signalSemaphoreInfoCount = 0;
		submitInfo.pSignalSemaphoreInfos = nullptr;
		ZE_VK_THROW_NOSUCC(vkQueueSubmit2(queue, 1, &submitInfo, VK_NULL_HANDLE));
	}

	U64 Device::SetFenceCPU(VkSemaphore fence, UA64& fenceVal)
	{
		ZE_VK_ENABLE();

		VkSemaphoreSignalInfo signalInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO, nullptr };
		signalInfo.semaphore = fence;
		signalInfo.value = ++fenceVal;
		ZE_VK_THROW_NOSUCC(vkSignalSemaphore(device, &signalInfo));
		return signalInfo.value;
	}

	U64 Device::SetFenceGPU(VkSemaphore fence, VkQueue queue, UA64& fenceVal)
	{
		// TODO: expose stageMask with own enum (no effect on D3D12)
		ZE_VK_ENABLE();

		VkSemaphoreSubmitInfo signalInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr };
		signalInfo.semaphore = fence;
		signalInfo.value = ++fenceVal;
		signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // First synch scope, signal after these commands
		signalInfo.deviceIndex = 0;

		VkSubmitInfo2 submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO_2, nullptr };
		submitInfo.flags = 0;
		submitInfo.waitSemaphoreInfoCount = 0;
		submitInfo.pWaitSemaphoreInfos = nullptr;
		submitInfo.commandBufferInfoCount = 0;
		submitInfo.pCommandBufferInfos = nullptr;
		submitInfo.signalSemaphoreInfoCount = 1;
		submitInfo.pSignalSemaphoreInfos = &signalInfo;
		ZE_VK_THROW_NOSUCC(vkQueueSubmit2(queue, 1, &submitInfo, VK_NULL_HANDLE));

		return signalInfo.value;
	}

	void Device::Execute(VkQueue queue, CommandList& cl)
	{
		ZE_ASSERT(cl.GetBuffer() != nullptr, "Empty list!");
		ZE_VK_ENABLE();

		VkCommandBufferSubmitInfo bufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, nullptr };
		bufferInfo.commandBuffer = cl.GetBuffer();
		bufferInfo.deviceMask = 0;

		VkSubmitInfo2 submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO_2, nullptr };
		submitInfo.flags = 0;
		submitInfo.waitSemaphoreInfoCount = 0;
		submitInfo.pWaitSemaphoreInfos = nullptr;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &bufferInfo;
		submitInfo.signalSemaphoreInfoCount = 0;
		submitInfo.pSignalSemaphoreInfos = nullptr;
		ZE_VK_THROW_NOSUCC(vkQueueSubmit2(queue, 1, &submitInfo, VK_NULL_HANDLE));
	}

#if _ZE_DEBUG_GFX_API
	VkBool32 VKAPI_PTR Device::DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		const bool attentionRequired = (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			&& (messageTypes & (VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT));
		Logger::InfoNoFile(pCallbackData->pMessage, attentionRequired);
		if (attentionRequired)
		{
			ZE_BREAK();
		}
		return VK_FALSE;
	}
#endif
#if _ZE_GFX_MARKERS
	void Device::BeingTag(VkQueue queue, std::string_view tag, Pixel color) noexcept
	{
		VkDebugUtilsLabelEXT labelInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, nullptr };
		labelInfo.pLabelName = tag.data();
		*reinterpret_cast<ColorF4*>(labelInfo.color) = { color.Red, color.Green, color.Blue, color.Alpha };
		vkQueueBeginDebugUtilsLabelEXT(queue, &labelInfo);
	}
#endif

	bool Device::FindQueueIndices(VkPhysicalDevice device, VkSurfaceKHR testSurface, QueueFamilyInfo& familyInfo) noexcept
	{
		U32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, nullptr);
		if (queueFamilyCount == 0)
			return false;

		std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount, { VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2, nullptr });
		vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, queueFamilies.data());

		std::bitset<3> supportedQueues = 0;
		for (U32 i = 0; i < queueFamilyCount; ++i)
		{
			const VkQueueFlags flags = queueFamilies.at(i).queueFamilyProperties.queueFlags;
			if (flags & VK_QUEUE_TRANSFER_BIT)
			{
				if (flags & VK_QUEUE_COMPUTE_BIT)
				{
					VkBool32 presentSupport = VK_FALSE;
					vkGetPhysicalDeviceSurfaceSupportKHR(device, i, testSurface, &presentSupport);
					if (flags & VK_QUEUE_GRAPHICS_BIT)
					{
						// Choose only gfx queue that supports presentation
						if (presentSupport)
						{
							supportedQueues[0] = true;
							familyInfo.Gfx = i;
						}
					}
					else
					{
						supportedQueues[1] = true;
						familyInfo.Compute = i;
						familyInfo.PresentFromCompute = presentSupport;
					}
				}
				else
				{
					supportedQueues[2] = true;
					familyInfo.Copy = i;
				}
			}
			if (supportedQueues.all())
				break;
		}
		return supportedQueues.all();
	}

	Device::GpuFitness Device::CheckGpuFitness(VkPhysicalDevice device, const VkPhysicalDeviceSurfaceInfo2KHR& testSurfaceInfo,
		const std::vector<const char*>& requiredExt, RequiredExtensionFeatures& features, QueueFamilyInfo& familyInfo)
	{
		ZE_VK_ENABLE();

		// Check if this GPU is enough for using in rendering
		vkGetPhysicalDeviceFeatures2(device, &features.Features);
		if (!features.TimelineSemaphore.timelineSemaphore || !features.Sync2.synchronization2
			|| !features.DynamicRendering.dynamicRendering || !features.Multiview.multiview
			|| !features.Features.features.geometryShader || !features.Features.features.samplerAnisotropy
			|| !features.Features.features.multiViewport || !features.PrimitiveRestart.primitiveTopologyListRestart
			|| !features.DynamicState.extendedDynamicState || !features.ImagelessFramebuffer.imagelessFramebuffer
			|| !features.DescriptorIndexing.shaderInputAttachmentArrayDynamicIndexing
			|| !features.DescriptorIndexing.shaderUniformTexelBufferArrayDynamicIndexing
			|| !features.DescriptorIndexing.shaderStorageTexelBufferArrayDynamicIndexing
			|| !features.DescriptorIndexing.shaderUniformBufferArrayNonUniformIndexing
			|| !features.DescriptorIndexing.shaderSampledImageArrayNonUniformIndexing
			|| !features.DescriptorIndexing.shaderStorageBufferArrayNonUniformIndexing
			|| !features.DescriptorIndexing.shaderStorageImageArrayNonUniformIndexing
			|| !features.DescriptorIndexing.shaderInputAttachmentArrayNonUniformIndexing
			|| !features.DescriptorIndexing.shaderUniformTexelBufferArrayNonUniformIndexing
			|| !features.DescriptorIndexing.shaderStorageTexelBufferArrayNonUniformIndexing
			|| !features.DescriptorIndexing.descriptorBindingUpdateUnusedWhilePending
			|| !features.DescriptorIndexing.descriptorBindingPartiallyBound
			|| !features.DescriptorIndexing.descriptorBindingVariableDescriptorCount
			|| !features.DescriptorIndexing.runtimeDescriptorArray
			|| !features.DescriptorBuffer.descriptorBuffer || !features.DescriptorBuffer.descriptorBufferPushDescriptors
			|| !features.BufferAddress.bufferDeviceAddress)
			return { GpuFitness::Status::FeaturesInsufficient };

		// Check if surface has any present modes
		U32 count = 0;
		ZE_VK_THROW_NOSUCC(vkGetPhysicalDeviceSurfacePresentModesKHR(device, testSurfaceInfo.surface, &count, nullptr));
		if (count == 0)
			return { GpuFitness::Status::NoPresentModes };

		// Check if surface has needed formats
		ZE_VK_THROW_NOSUCC(vkGetPhysicalDeviceSurfaceFormats2KHR(device, &testSurfaceInfo, &count, nullptr));
		if (count == 0)
			return { GpuFitness::Status::NoPixelFormats };
		std::vector<VkSurfaceFormat2KHR> supportedFormats(count, { VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR, nullptr });
		ZE_VK_THROW_NOSUCC(vkGetPhysicalDeviceSurfaceFormats2KHR(device, &testSurfaceInfo, &count, supportedFormats.data()));
		bool notSupported = true;
		for (const VkSurfaceFormat2KHR& format : supportedFormats)
		{
			if (Utils::IsSameFormatFamily(GetFormatFromVk(format.surfaceFormat.format), Settings::GetBackbufferFormat()))
			{
				notSupported = false;
				break;
			}
		}
		if (notSupported)
			return { GpuFitness::Status::BackbufferFormatNotSupported };

		// Check for needed queues
		if (!FindQueueIndices(device, testSurfaceInfo.surface, familyInfo))
			return { GpuFitness::Status::QueuesInsufficient };

		// Make sure that all required extensions are present
		ZE_VK_THROW_NOSUCC(vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr));
		if (count == 0)
			return { GpuFitness::Status::NoExtensions };
		std::vector<VkExtensionProperties> supportedExtensions(count);
		ZE_VK_THROW_NOSUCC(vkEnumerateDeviceExtensionProperties(device, nullptr, &count, supportedExtensions.data()));
		for (U32 i = 0; i < requiredExt.size(); ++i)
		{
			bool notFound = true;
			for (const VkExtensionProperties& ext : supportedExtensions)
			{
				if (strcmp(ext.extensionName, requiredExt.at(i)) == 0)
				{
					notFound = false;
					break;
				}
			}
			if (notFound)
				return { GpuFitness::Status::NotAllRequiredExtSupported, i };
		}

		return { GpuFitness::Status::Good };
	}

	void Device::InitVolk()
	{
		ZE_WARNING_PUSH
			ZE_WARNING_DISABLE_MSVC(4191) // GetProcAddress() call causes this
			ZE_WARNING_DISABLE_MSVC(5039) // MSVC don't like volkInitializeCustom()

			// Custom way of initializing Vulkan in order to correctly unload library
			PFN_vkGetInstanceProcAddr getInstanceAddr;
#if _ZE_PLATFORM_WINDOWS
		HMODULE lib = LoadLibraryW(L"vulkan-1.dll");
		if (!lib)
			throw ZE_CMP_EXCEPT("Vulkan library missing! Cannot find [vulkan-1.dll]!");

		getInstanceAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(lib, "vkGetInstanceProcAddr"));
#else
		void* lib = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
		if (!lib)
			lib = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
		if (!lib)
			throw ZE_CMP_EXCEPT("Vulkan library missing! Cannot find [libvulkan.so.1] or [libvulkan.so]!");

		getInstanceAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(lib, "vkGetInstanceProcAddr"));
#endif
		vulkanLibModule = static_cast<LibraryHandle>(lib);
		volkInitializeCustom(getInstanceAddr);
		ZE_WARNING_POP
	}

	void Device::CreateInstance()
	{
		ZE_VK_ENABLE();

		VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr };
		appInfo.pApplicationName = Settings::GetAppName();
		appInfo.applicationVersion = Settings::GetAppVersion();
		appInfo.pEngineName = Settings::GetEngineName();
		appInfo.engineVersion = Settings::GetEngineVersion();
		appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

		// Check for supported API version
		U32 instanceVersion = 0;
		ZE_VK_THROW_NOSUCC(vkEnumerateInstanceVersion(&instanceVersion));
		if (instanceVersion < appInfo.apiVersion)
		{
			throw ZE_CMP_EXCEPT("Vulkan instance version reported is not supported on this engine!\nRequired version: "
				+ std::to_string(appInfo.apiVersion) + "\nPresent version: " + std::to_string(instanceVersion));
		}

		// Check for instance extensions
		U32 count = 0;
		ZE_VK_THROW_NOSUCC(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
		if (count == 0)
			throw ZE_CMP_EXCEPT("No Vulkan instance extensions present!");
		std::vector<VkExtensionProperties> supportedExtensions(count);
		ZE_VK_THROW_NOSUCC(vkEnumerateInstanceExtensionProperties(nullptr, &count, supportedExtensions.data()));

#define X(ext) ext,
		std::vector<const char*> enabledExtensions
		{
#if _ZE_DEBUG_GFX_API
			ZE_VK_EXT_LIST_INSTANCE_DEBUG
#endif
#if _ZE_GFX_MARKERS
			X(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
#endif
#if !_ZE_PLATFORM_LINUX
			ZE_VK_EXT_LIST_INSTANCE_PLATFORM_REQUIRED
#endif
			ZE_VK_EXT_LIST_INSTANCE_REQUIRED
		};
#undef X

#define X(ext) GetExtensionIndex(ext),
		std::vector<U16> extensionsIndices
		{
#if _ZE_DEBUG_GFX_API
			ZE_VK_EXT_LIST_INSTANCE_DEBUG
#endif
#if _ZE_GFX_MARKERS
			X(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
#endif
#if !_ZE_PLATFORM_LINUX
			ZE_VK_EXT_LIST_INSTANCE_PLATFORM_REQUIRED
#endif
			ZE_VK_EXT_LIST_INSTANCE_REQUIRED
		};
#undef X

		auto isExtSupported = [&supportedExtensions](const char* name) -> bool
		{
			for (const VkExtensionProperties& ext : supportedExtensions)
				if (strcmp(ext.extensionName, name) == 0)
					return true;
			return false;
		};
		// Sometimes validation features extension is not reported but is present when enabled with validation layer
		for (U32 i = _ZE_DEBUG_GFX_API; i < enabledExtensions.size(); ++i)
		{
			if (!isExtSupported(enabledExtensions.at(i)))
				throw ZE_CMP_EXCEPT("Required instance extension is not supported: " + std::string(enabledExtensions.at(i)));
			extensionSupport[extensionsIndices.at(i)] = true;
		}

		// Enable all optional extensions that are present
#define X(ext) ext,
		std::vector<const char*> optionalExtensions{ ZE_VK_EXT_LIST_INSTANCE_OPTIONAL };
#undef X
#define X(ext) GetExtensionIndex(ext),
		extensionsIndices = { ZE_VK_EXT_LIST_INSTANCE_OPTIONAL };
#undef X
		for (U32 i = 0; i < optionalExtensions.size(); ++i)
		{
			if (isExtSupported(optionalExtensions.at(i)))
			{
				extensionSupport[extensionsIndices.at(i)] = true;
				enabledExtensions.emplace_back(optionalExtensions.at(i));
			}
		}
		extensionsIndices.clear();
		optionalExtensions.clear();
		supportedExtensions.clear();

#if _ZE_PLATFORM_LINUX
		const char* linuxSurface = nullptr;
		std::bitset<4> linuxSurfaceSupport = 0;
		// Find correct supported surface type
		for (U32 i = 0; i < extensionCount; ++i)
		{
			if (!linuxSurfaceSupport[0])
				linuxSurfaceSupport[0] = strcmp(supportedExtensions[i].extensionName, VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME) == 0;
			if (!linuxSurfaceSupport[1])
				linuxSurfaceSupport[1] = strcmp(supportedExtensions[i].extensionName, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME) == 0;
			if (!linuxSurfaceSupport[2])
				linuxSurfaceSupport[2] = strcmp(supportedExtensions[i].extensionName, VK_KHR_XCB_SURFACE_EXTENSION_NAME) == 0;
			if (!linuxSurfaceSupport[3])
				linuxSurfaceSupport[3] = strcmp(supportedExtensions[i].extensionName, VK_KHR_XLIB_SURFACE_EXTENSION_NAME) == 0;

			if (linuxSurfaceSupport.all())
				break;
		}
		if (!linuxSurfaceSupport.any())
			throw ZE_CMP_EXCEPT("No extensions to support surface creation on this platform!");

		if (linuxSurfaceSupport[1])
		{
			enabledExtensions.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
			extensionSupport[GetExtensionIndex(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME)] = true;
		}
		else if (linuxSurfaceSupport[0])
		{
			enabledExtensions.emplace_back(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);
			extensionSupport[GetExtensionIndex(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME)] = true;
		}
		else if (linuxSurfaceSupport[2])
		{
			enabledExtensions.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
			extensionSupport[GetExtensionIndex(VK_KHR_XCB_SURFACE_EXTENSION_NAME)] = true;
		}
		else
		{
			enabledExtensions.emplace_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
			extensionSupport[GetExtensionIndex(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)] = true;
		}
#endif

		// Prepare instace options
		VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr };
		instanceInfo.flags = 0;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledExtensionCount = Utils::SafeCast<U32>(enabledExtensions.size());
		instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();

		// Check for desired layers
		const char* enabledLayers[]
		{
			"VK_LAYER_KHRONOS_synchronization2",
#if _ZE_DEBUG_GFX_API
			"VK_LAYER_KHRONOS_validation"
#endif
		};
		instanceInfo.enabledLayerCount = sizeof(enabledLayers) / sizeof(const char*);
		instanceInfo.ppEnabledLayerNames = enabledLayers;

		ZE_VK_THROW_NOSUCC(vkEnumerateInstanceLayerProperties(&count, nullptr));
		if (count == 0)
			throw ZE_CMP_EXCEPT("No Vulkan layers present!");
		std::vector<VkLayerProperties> presentLayerProperties(count);
		ZE_VK_THROW_NOSUCC(vkEnumerateInstanceLayerProperties(&count, presentLayerProperties.data()));

		for (U32 i = 0; i < instanceInfo.enabledLayerCount; ++i)
		{
			bool notPresent = true;
			for (const VkLayerProperties& layer : presentLayerProperties)
			{
				if (strcmp(layer.layerName, enabledLayers[i]) == 0)
				{
					notPresent = false;
					break;
				}
			}
			if (notPresent)
				throw ZE_CMP_EXCEPT("Required layer is not supported: " + std::string(enabledLayers[i]));
		}
		presentLayerProperties.clear();

		// Specify debug features
#if _ZE_DEBUG_GFX_API
		VkValidationFeaturesEXT validationFeatures = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, nullptr };
		validationFeatures.disabledValidationFeatureCount = 0;
		validationFeatures.pDisabledValidationFeatures = nullptr;

		VkDebugUtilsMessengerCreateInfoEXT debugMessenger = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, &validationFeatures };
		debugMessenger.flags = 0;
		debugMessenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugMessenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		// if VK_EXT_device_address_binding_report then VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT
		debugMessenger.pfnUserCallback = DebugMessengerCallback;
		debugMessenger.pUserData = nullptr;
		instanceInfo.pNext = &debugMessenger;

		const VkValidationFeatureEnableEXT enabledValidations[] =
		{
#if _ZE_DEBUG_GPU_VALIDATION
			VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
			VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
#else
			VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
#endif
			VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
			VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
		};
		validationFeatures.enabledValidationFeatureCount = sizeof(enabledValidations) / sizeof(VkValidationFeatureEnableEXT);
		validationFeatures.pEnabledValidationFeatures = enabledValidations;
#endif

		// Create instance
		ZE_VK_THROW_NOSUCC(vkCreateInstance(&instanceInfo, nullptr, &instance));
		volkLoadInstanceOnly(instance);
	}

	void Device::FindPhysicalDevice(const std::vector<const char*>& requiredExt,
		const Window::MainWindow& window, RequiredExtensionFeatures& features)
	{
		ZE_VK_ENABLE();

		// Get present GPUs available in system
		U32 deviceCount = 0;
		ZE_VK_THROW_NOSUCC(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
		if (deviceCount == 0)
			throw ZE_CMP_EXCEPT("Current system have no avaiable GPUs!");

		VkPhysicalDeviceDescriptorBufferPropertiesEXT descBufferProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT, nullptr };
		VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT, &descBufferProperties };
		VkPhysicalDeviceProperties2 properties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &conservativeRasterProperties };
		QueueFamilyInfo familyInfo = {};
		// Check for support of presentation on given queue
		// VkSurfaceFullScreenExclusiveInfoEXT or VkSurfaceFullScreenExclusiveWin32InfoEXT
		VkPhysicalDeviceSurfaceInfo2KHR testSurfaceInfo = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, nullptr, CreateSurface(window, instance) };
		if (deviceCount == 1)
		{
			// Only 1 GPU present so can skip all the hassle
			ZE_VK_THROW_NOSUCC(vkEnumeratePhysicalDevices(instance, &deviceCount, &physicalDevice));
			vkGetPhysicalDeviceProperties2(physicalDevice, &properties);

			const GpuFitness fit = descBufferProperties.bufferlessPushDescriptors ? CheckGpuFitness(physicalDevice, testSurfaceInfo, requiredExt, features, familyInfo) : GpuFitness{ GpuFitness::Status::FeaturesInsufficient };
			if (fit.Status != GpuFitness::Status::Good)
			{
				switch (fit.Status)
				{
				default:
					ZE_ENUM_UNHANDLED();
				case GpuFitness::Status::FeaturesInsufficient:
					throw ZE_CMP_EXCEPT("GPU [" + std::string(properties.properties.deviceName) + "] doesn't support enough basic Vulkan features!");
				case GpuFitness::Status::NoPresentModes:
					throw ZE_CMP_EXCEPT("Window surface doesn't support any present modes on GPU [" + std::string(properties.properties.deviceName) + "]!");
				case GpuFitness::Status::NoPixelFormats:
					throw ZE_CMP_EXCEPT("Window surface doesn't support any pixel formats on GPU [" + std::string(properties.properties.deviceName) + "]!");
				case GpuFitness::Status::BackbufferFormatNotSupported:
					throw ZE_CMP_EXCEPT("Window surface doesn't support required backbuffer formaton GPU [" + std::string(properties.properties.deviceName) + "]! Format: [" + std::string(Utils::FormatToString(Settings::GetBackbufferFormat())) + "]");
				case GpuFitness::Status::QueuesInsufficient:
					throw ZE_CMP_EXCEPT("GPU [" + std::string(properties.properties.deviceName) + "] doesn't support all of the graphics, compute and copy queues!");
				case GpuFitness::Status::NoExtensions:
					throw ZE_CMP_EXCEPT("No Vulkan device extensions present on GPU [" + std::string(properties.properties.deviceName) + "]!");
				case GpuFitness::Status::NotAllRequiredExtSupported:
					throw ZE_CMP_EXCEPT("GPU [" + std::string(properties.properties.deviceName) + "] doesn't support required device extension: " + std::string(requiredExt.at(fit.ExtIndex)));
				}
			}
		}
		else
		{
			std::vector<VkPhysicalDevice> devices(deviceCount);
			ZE_VK_THROW_NOSUCC(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

			// Get most suited GPU for rendering using some ranking algorithm
			std::multimap<U16, std::pair<VkPhysicalDevice, QueueFamilyInfo>> deviceRank;
			for (VkPhysicalDevice dev : devices)
			{
				if (CheckGpuFitness(dev, testSurfaceInfo, requiredExt, features, familyInfo).Status != GpuFitness::Status::Good)
					continue;

				U16 rank = 0;
				vkGetPhysicalDeviceProperties2(dev, &properties);
				if (!descBufferProperties.bufferlessPushDescriptors)
					continue;
				switch (properties.properties.deviceType)
				{
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				{
					rank += 40000;
					break;
				}
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				{
					// Might put it below iGPU
					rank += 30000;
					break;
				}
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				{
					rank += 20000;
					break;
				}
				case VK_PHYSICAL_DEVICE_TYPE_CPU:
				{
					rank += 10000;
					break;
				}
				default:
					ZE_ENUM_UNHANDLED();
				case VK_PHYSICAL_DEVICE_TYPE_OTHER:
					break;
				}

				// Favor lower memory granularity
				rank += Utils::SafeCast<U16>(properties.properties.limits.bufferImageGranularity / 100);
				// Smaller coherent memory chunks are better for aligning of mappable memory (HOST_VISIBLE without HOST_COHERENT)
				rank += Utils::SafeCast<U16>(properties.properties.limits.nonCoherentAtomSize / 50);

				// Handle GPU memory as one of the main indicators of performance
				VkPhysicalDeviceMemoryProperties2 memoryProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2, nullptr };
				vkGetPhysicalDeviceMemoryProperties2(dev, &memoryProps);
				for (U32 i = 0; i < memoryProps.memoryProperties.memoryHeapCount; ++i)
				{
					if (memoryProps.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
						rank += Utils::SafeCast<U16>(Math::DivideRoundUp(memoryProps.memoryProperties.memoryHeaps[i].size, Math::GIGABYTE) * 100);
				}
				deviceRank.emplace(rank, std::make_pair(dev, familyInfo));
			}
			devices.clear();

			// Sanity check whether we have enough GPUs to choose from
			if (deviceRank.size() == 0)
				throw ZE_CMP_EXCEPT("None of the GPUs support required Vulkan features!");

			// Get one with highest score
			physicalDevice = deviceRank.rbegin()->second.first;
			familyInfo = deviceRank.rbegin()->second.second;

			vkGetPhysicalDeviceProperties2(physicalDevice, &properties);
			vkGetPhysicalDeviceFeatures2(physicalDevice, &features.Features);
		}
		vkDestroySurfaceKHR(instance, testSurfaceInfo.surface, nullptr);

		// Mark all required extensions as present
#define X(ext) GetExtensionIndex(ext),
		std::vector<U16> enabledExtIndices
		{
#if _ZE_DEBUG_GFX_API
			ZE_VK_EXT_LIST_DEVICE_DEBUG
#endif
			ZE_VK_EXT_LIST_DEVICE_REQUIRED
		};
#undef X
		for (U32 i = 0; i < enabledExtIndices.size(); ++i)
			extensionSupport[enabledExtIndices.at(i)] = true;

		// Setup all queue indices
		gfxQueueIndex = familyInfo.Gfx;
		computeQueueIndex = familyInfo.Compute;
		copyQueueIndex = familyInfo.Copy;
		limits = properties.properties.limits;
		descBufferAlignment = descBufferProperties.descriptorBufferOffsetAlignment;
		conservativeRasterOverestimateSize = conservativeRasterProperties.primitiveOverestimationSize;
		SetIntegratedGPU(properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
		SetPresentFromComputeSupport(familyInfo.PresentFromCompute);
	}

	Device::Device(const Window::MainWindow& window, U32 descriptorCount, U32 scratchDescriptorCount)
	{
		ZE_VK_ENABLE_ID();
		InitVolk();
		CreateInstance();

		VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV dedicatedAllocAliasing = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV, nullptr, VK_FALSE };
		VkPhysicalDeviceCoherentMemoryFeaturesAMD coherentMemory = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD, nullptr, VK_FALSE };
		VkPhysicalDeviceMemoryPriorityFeaturesEXT memPriority = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT };
		switch (Settings::GetGpuVendor())
		{
		case GFX::VendorGPU::AMD:
		{
			memPriority.pNext = &coherentMemory;
			break;
		}
		case GFX::VendorGPU::Nvidia:
		{
			memPriority.pNext = &dedicatedAllocAliasing;
			break;
		}
		default:
		{
			memPriority.pNext = nullptr;
			break;
		}
		}
		VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT pageableMemory = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT, &memPriority };
		VkPhysicalDeviceIndexTypeUint8FeaturesEXT indicesU8 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT, &pageableMemory };
		VkPhysicalDeviceSubgroupSizeControlFeatures subgroupControl = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES, &indicesU8 };
		VkPhysicalDeviceVulkanMemoryModelFeatures memoryModel = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES, &subgroupControl };

		RequiredExtensionFeatures requiredFeatures = {};
		requiredFeatures.BufferAddress = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, &memoryModel };
		requiredFeatures.DescriptorBuffer = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT, &requiredFeatures.BufferAddress };
		requiredFeatures.DescriptorIndexing = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, &requiredFeatures.DescriptorBuffer };
		requiredFeatures.ImagelessFramebuffer = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES, &requiredFeatures.DescriptorIndexing };
		requiredFeatures.DynamicState = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT, &requiredFeatures.ImagelessFramebuffer };
		requiredFeatures.PrimitiveRestart = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT, &requiredFeatures.DynamicState };
		requiredFeatures.Multiview = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES, &requiredFeatures.PrimitiveRestart };
		requiredFeatures.DynamicRendering = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES, &requiredFeatures.Multiview };
		requiredFeatures.Sync2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES, &requiredFeatures.DynamicRendering };
		requiredFeatures.TimelineSemaphore = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES, &requiredFeatures.Sync2 };
		requiredFeatures.Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &requiredFeatures.TimelineSemaphore };

#define X(ext) ext,
		std::vector<const char*> enabledExtensions
		{
#if _ZE_DEBUG_GFX_API
			ZE_VK_EXT_LIST_DEVICE_DEBUG
#endif
			ZE_VK_EXT_LIST_DEVICE_REQUIRED
		};
#undef X
		FindPhysicalDevice(enabledExtensions, window, requiredFeatures);

		// Enable all optional extensions that are present
#define X(ext) ext,
		std::vector<const char*> optionalExtensions{ ZE_VK_EXT_LIST_DEVICE_OPTIONAL };
#undef X
#define X(ext) GetExtensionIndex(ext),
		std::vector<U16> extensionsIndices{ ZE_VK_EXT_LIST_DEVICE_OPTIONAL };
#undef X
		U32 count = 0;
		ZE_VK_THROW_NOSUCC(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr));
		ZE_ASSERT(count != 0, "There should be always some extensions in picked physical device!");
		std::vector<VkExtensionProperties> supportedExtensions(count);
		ZE_VK_THROW_NOSUCC(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, supportedExtensions.data()));

		for (U32 i = 0; i < optionalExtensions.size(); ++i)
		{
			for (const VkExtensionProperties& ext : supportedExtensions)
			{
				if (strcmp(ext.extensionName, optionalExtensions.at(i)) == 0)
				{
					extensionSupport[extensionsIndices.at(i)] = true;
					enabledExtensions.emplace_back(optionalExtensions.at(i));
					break;
				}
			}
		}
		supportedExtensions.clear();
		extensionsIndices.clear();
		optionalExtensions.clear();

		// Enable features of the extensions
		extensionSupport[GetExtensionIndex(VK_NV_DEDICATED_ALLOCATION_IMAGE_ALIASING_EXTENSION_NAME)] = dedicatedAllocAliasing.dedicatedAllocationImageAliasing;
		extensionSupport[GetExtensionIndex(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME)] = coherentMemory.deviceCoherentMemory;
		extensionSupport[GetExtensionIndex(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME)] = memPriority.memoryPriority;
		extensionSupport[GetExtensionIndex(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME)] = pageableMemory.pageableDeviceLocalMemory;
		Settings::SetU8IndexSets(extensionSupport[GetExtensionIndex(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME)] = indicesU8.indexTypeUint8);
		if (extensionSupport[GetExtensionIndex(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME)] = subgroupControl.subgroupSizeControl)
			SetFullComputeSubgroupSupport(subgroupControl.computeFullSubgroups);
		if (extensionSupport[GetExtensionIndex(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME)] = memoryModel.vulkanMemoryModel)
		{
			SetMemoryModelDeviceScope(memoryModel.vulkanMemoryModelDeviceScope);
			SetMemoryModelChains(memoryModel.vulkanMemoryModelAvailabilityVisibilityChains);
		}
		SetMultiviewGeometryShaderSupport(requiredFeatures.Multiview.multiviewGeometryShader);
		SetMultiviewTesselationSupport(requiredFeatures.Multiview.multiviewTessellationShader);
		SetPatchListIndexRestartSupport(requiredFeatures.PrimitiveRestart.primitiveTopologyPatchListRestart);
		SetDescriptorBufferImageLayoutIngored(requiredFeatures.DescriptorBuffer.descriptorBufferImageLayoutIgnored);
		SetDescriptorBufferCaptureReplaySupported(requiredFeatures.DescriptorBuffer.descriptorBufferCaptureReplay);
		SetBufferCaptureReplaySupported(requiredFeatures.BufferAddress.bufferDeviceAddressCaptureReplay);
		SetBufferAddressMultiDeviceSupported(requiredFeatures.BufferAddress.bufferDeviceAddressMultiDevice);

		// Setup known features
		requiredFeatures.Features.features.independentBlend = VK_FALSE;

		// Describe used queues
		const float queuePriority = 1.0f;
		const VkDeviceQueueCreateInfo queueInfos[3]
		{
			{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, gfxQueueIndex, 1, &queuePriority },
			{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, computeQueueIndex, 1, &queuePriority },
			{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, copyQueueIndex, 1, &queuePriority }
		};

		// Control overallocation behavior on AMD cards
		VkDeviceMemoryOverallocationCreateInfoAMD overallocBehavior = { VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD, nullptr };
		if (IsExtensionSupported(VK_AMD_MEMORY_OVERALLOCATION_BEHAVIOR_EXTENSION_NAME))
		{
			overallocBehavior.overallocationBehavior = VK_MEMORY_OVERALLOCATION_BEHAVIOR_DISALLOWED_AMD;
			coherentMemory.pNext = &overallocBehavior;
		}

		// Create logic device
		VkDeviceGroupDeviceCreateInfo deviceGroupInfo = { VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO, &requiredFeatures.Features };
		deviceGroupInfo.physicalDeviceCount = 1;
		deviceGroupInfo.pPhysicalDevices = &physicalDevice;

		VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, &deviceGroupInfo };
		deviceInfo.flags = 0;
		deviceInfo.queueCreateInfoCount = sizeof(queueInfos) / sizeof(VkDeviceQueueCreateInfo);
		deviceInfo.pQueueCreateInfos = queueInfos;
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
		deviceInfo.enabledExtensionCount = Utils::SafeCast<U32>(enabledExtensions.size());
		deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
		deviceInfo.pEnabledFeatures = nullptr;

		ZE_VK_THROW_NOSUCC(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device));
		volkLoadDevice(device);

		ZE_VK_SET_ID(device, device, VK_OBJECT_TYPE_DEVICE, "main_device");
		ZE_VK_SET_ID(device, physicalDevice, VK_OBJECT_TYPE_PHYSICAL_DEVICE, "selected_gpu");

		vkGetDeviceQueue(device, gfxQueueIndex, 0, &gfxQueue);
		vkGetDeviceQueue(device, computeQueueIndex, 0, &computeQueue);
		vkGetDeviceQueue(device, copyQueueIndex, 0, &copyQueue);
		ZE_VK_SET_ID(device, gfxQueue, VK_OBJECT_TYPE_QUEUE, "gfx_queue");
		ZE_VK_SET_ID(device, computeQueue, VK_OBJECT_TYPE_QUEUE, "compute_queue");
		ZE_VK_SET_ID(device, copyQueue, VK_OBJECT_TYPE_QUEUE, "copy_queue");

		VkSemaphoreTypeCreateInfo timelineSemaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO, nullptr };
		timelineSemaphoreInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		timelineSemaphoreInfo.initialValue = 0;
		const VkSemaphoreCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, &timelineSemaphoreInfo, 0 };
		ZE_VK_THROW_NOSUCC(vkCreateSemaphore(device, &fenceInfo, nullptr, &gfxFence));
		ZE_VK_THROW_NOSUCC(vkCreateSemaphore(device, &fenceInfo, nullptr, &computeFence));
		ZE_VK_THROW_NOSUCC(vkCreateSemaphore(device, &fenceInfo, nullptr, &copyFence));
		ZE_VK_SET_ID(device, gfxFence, VK_OBJECT_TYPE_SEMAPHORE, "gfx_fence");
		ZE_VK_SET_ID(device, computeFence, VK_OBJECT_TYPE_SEMAPHORE, "compute_fence");
		ZE_VK_SET_ID(device, copyFence, VK_OBJECT_TYPE_SEMAPHORE, "copy_fence");

		allocator.Init(*this);

		copyList.Init(*this, GFX::QueueType::Main);
		copyResInfo.Size = 0;
		copyResInfo.Allocated = COPY_LIST_GROW_SIZE;
	}

	Device::~Device()
	{
		ZE_ASSERT(copyResInfo.Size == 0, "Copying data not finished before destroying Device!");

		copyList.Free(*this);
		if (commandLists)
			commandLists.Free();
		allocator.Destroy(*this);
		if (device)
			vkDestroyDevice(device, nullptr);
		if (instance)
			vkDestroyInstance(instance, nullptr);
#if _ZE_PLATFORM_WINDOWS
		if (vulkanLibModule)
		{
			[[maybe_unused]] const BOOL status = FreeLibrary(vulkanLibModule.CastPtr<HMODULE>());
			ZE_ASSERT(status, "Error unloading Vulkan library!");
		}
#else
		if (vulkanLibModule)
		{
			S32 status = dlclose(vulkanLibModule);
			ZE_ASSERT(status == 0, "Error unloading Vulkan library!");
		}
#endif
	}

	void Device::BeginUploadRegion()
	{
		ZE_ASSERT(copyResList == nullptr, "Finish previous upload first!");
		copyList.Open();
		copyResList = Table::Create<UploadInfo>(COPY_LIST_GROW_SIZE);
	}

	void Device::StartUpload()
	{
		ZE_ASSERT(copyResList != nullptr, "Empty upload list!");

		U16 size = copyResInfo.Size - copyOffset;
		if (size)
		{
			VkDependencyInfo depInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO, nullptr };
			depInfo.dependencyFlags = 0;
			depInfo.memoryBarrierCount = 0;
			depInfo.pMemoryBarriers = nullptr;

			std::vector<VkBufferMemoryBarrier2> bufferBarriers;
			std::vector<VkImageMemoryBarrier2> imageBarriers;
			for (U16 i = 0; i < size; ++i)
			{
				UploadInfo& copyInfo = copyResList[copyOffset + i];
				if (copyInfo.IsBuffer)
				{
					VkBufferMemoryBarrier2& barrier = bufferBarriers.emplace_back(VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2, nullptr);
					barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
					barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
					barrier.dstStageMask = copyInfo.DestStage;
					barrier.dstAccessMask = copyInfo.DestAccess;
					barrier.srcQueueFamilyIndex = gfxQueueIndex;
					barrier.dstQueueFamilyIndex = copyInfo.LastUsedQueue;
					barrier.buffer = copyInfo.DestBuffer;
					barrier.offset = 0;
					barrier.size = VK_WHOLE_SIZE;
				}
				else
				{
					VkImageMemoryBarrier2& barrier = imageBarriers.emplace_back(VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, nullptr);
					barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
					barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
					barrier.dstStageMask = copyInfo.DestStage;
					barrier.dstAccessMask = copyInfo.DestAccess;
					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR;
					barrier.srcQueueFamilyIndex = gfxQueueIndex;
					barrier.dstQueueFamilyIndex = copyInfo.LastUsedQueue;
					barrier.image = copyInfo.DestImage;
					barrier.subresourceRange;
				}
			}
			depInfo.bufferMemoryBarrierCount = Utils::SafeCast<U32>(bufferBarriers.size());
			depInfo.pBufferMemoryBarriers = bufferBarriers.data();
			depInfo.imageMemoryBarrierCount = Utils::SafeCast<U32>(imageBarriers.size());
			depInfo.pImageMemoryBarriers = imageBarriers.data();

			copyOffset = copyResInfo.Size;
			vkCmdPipelineBarrier2(copyList.GetBuffer(), &depInfo);

			copyList.Close();
			Execute(gfxQueue, copyList);
			copyList.Open();
		}
	}

	void Device::EndUploadRegion()
	{
		ZE_ASSERT(copyResList != nullptr, "Start upload region first!");
		ZE_VK_ENABLE();

		copyList.Close();
		ZE_VK_THROW_NOSUCC(vkQueueWaitIdle(gfxQueue));
		copyList.Reset(*this);

		for (U16 i = 0; i < copyResInfo.Size; ++i)
		{
			allocator.Remove(*this, copyResList[i].StagingAlloc);
			if (copyResList[i].IsBuffer)
				vkDestroyBuffer(device, copyResList[i].StagingBuffer, nullptr);
			else
				vkDestroyImage(device, copyResList[i].StagingImage, nullptr);
		}
		Table::Clear(copyResInfo.Size, copyResList);
		copyOffset = 0;
		copyResInfo.Size = 0;
		copyResInfo.Allocated = COPY_LIST_GROW_SIZE;
	}

	void Device::Execute(GFX::CommandList* cls, U32 count)
	{
		if (count == 1)
		{
			const U32 family = cls->Get().vk.GetFamily();

			if (family == gfxQueueIndex)
				return ExecuteMain(*cls);
			if (family == computeQueueIndex)
				return ExecuteCompute(*cls);
			ZE_ASSERT(family == copyQueueIndex, "Incorrect type of command list!!!");
			return ExecuteCopy(*cls);
		}

		// Find max size for command lists to execute at once
		U32 gfxCount = 0, computeCount = 0, copyCount = 0;
		for (U32 i = 0; i < count; ++i)
		{
			ZE_ASSERT(cls[i].Get().vk.GetBuffer() != nullptr, "Empty list!");

			const U32 family = cls[i].Get().vk.GetFamily();

			if (family == gfxQueueIndex)
				++gfxCount;
			else if (family == computeQueueIndex)
				++computeCount;
			else
			{
				ZE_ASSERT(family == copyQueueIndex, "Incorrect type of command list!!!");
				++copyCount;
			}
		}

		// Realloc if needed bigger list
		count = std::max(commandListsCount, gfxCount);
		if (computeCount > count)
			count = computeCount;
		if (copyCount > count)
			count = copyCount;
		if (count > commandListsCount)
		{
			commandListsCount = count;
			commandLists = reinterpret_cast<VkCommandBufferSubmitInfo*>(realloc(commandLists, count * sizeof(VkCommandBufferSubmitInfo)));
		}

		VkSubmitInfo2 submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO_2, nullptr };
		submitInfo.flags = 0;
		submitInfo.waitSemaphoreInfoCount = 0;
		submitInfo.pWaitSemaphoreInfos = nullptr;
		submitInfo.pCommandBufferInfos = commandLists;
		submitInfo.signalSemaphoreInfoCount = 0;
		submitInfo.pSignalSemaphoreInfos = nullptr;

		// Execute lists
		auto submitQueue = [&](U32 count, U32 queueIndex, VkQueue queue)
		{
			if (count)
			{
				ZE_VK_ENABLE();

				submitInfo.commandBufferInfoCount = count;
				U32 i = 0, j = 0;
				do
				{
					if (cls[i].Get().vk.GetFamily() == queueIndex)
					{
						commandLists[i] = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, nullptr };
						commandLists[i].commandBuffer = cls[j].Get().vk.GetBuffer();
						commandLists[i++].deviceMask = 0;
					}
					++j;
				} while (i < count);
				ZE_VK_THROW_NOSUCC(vkQueueSubmit2(queue, 1, &submitInfo, VK_NULL_HANDLE));
			}
		};
		submitQueue(gfxCount, gfxQueueIndex, gfxQueue);
		submitQueue(computeCount, computeQueueIndex, computeQueue);
		submitQueue(copyCount, copyQueueIndex, copyQueue);
	}

	void Device::ExecuteMain(GFX::CommandList& cl)
	{
		Execute(gfxQueue, cl.Get().vk);
	}

	void Device::ExecuteCompute(GFX::CommandList& cl)
	{
		Execute(computeQueue, cl.Get().vk);
	}

	void Device::ExecuteCopy(GFX::CommandList& cl)
	{
		Execute(copyQueue, cl.Get().vk);
	}

	void Device::EndFrame() noexcept
	{
		allocator.HandleBudget(*this);
	}

	void Device::UploadBindBuffer(UploadInfoBuffer& uploadInfo)
	{
		ZE_VK_ENABLE_ID();
		ZE_ASSERT(copyResList != nullptr, "Empty upload list!");
		ZE_ASSERT(uploadInfo.InitData != nullptr, "Empty initial data!");

		const U32 deviceIndex = 0;
		VkBindBufferMemoryDeviceGroupInfo deviceGroupInfo = { VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO, nullptr };
		deviceGroupInfo.deviceIndexCount = 1;
		deviceGroupInfo.pDeviceIndices = &deviceIndex;

		uploadInfo.CreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		uploadInfo.Staging.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
		uploadInfo.Staging.pNext = &deviceGroupInfo;
		ZE_VK_THROW_NOSUCC(vkCreateBuffer(device, &uploadInfo.CreateInfo, nullptr, &uploadInfo.Staging.buffer));
		ZE_VK_SET_ID(device, uploadInfo.Staging.buffer, VK_OBJECT_TYPE_BUFFER,
			"Staging buffer [size:" + std::to_string(uploadInfo.CreateInfo.size) + "]");

		Allocation stagingAlloc = allocator.AllocBuffer(*this, uploadInfo.Staging.buffer, Allocation::Usage::StagingToGPU);
		U8* mappedMemory = nullptr;
		allocator.GetAllocInfo(stagingAlloc, uploadInfo.Staging.memoryOffset, uploadInfo.Staging.memory, &mappedMemory);

		ZE_VK_EXCEPT_RESULT = vkBindBufferMemory2(device, 2, &uploadInfo.Dest);
		uploadInfo.Staging.pNext = nullptr; // Clear pNext to avoid exposing current stack's data
		if (ZE_VK_EXCEPT_RESULT != VK_SUCCESS)
		{
			ZE_BREAK();
			throw ZE_VK_EXCEPT(ZE_VK_EXCEPT_RESULT);
		}

		ZE_ASSERT(mappedMemory != nullptr, "Staging buffer always should be accessible from CPU!");
		memcpy(mappedMemory, uploadInfo.InitData, uploadInfo.CreateInfo.size);

		VkBufferCopy2 bufferCopy = { VK_STRUCTURE_TYPE_BUFFER_COPY_2, nullptr };
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;
		bufferCopy.size = uploadInfo.CreateInfo.size;

		VkCopyBufferInfo2 copyInfo = { VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2, nullptr };
		copyInfo.srcBuffer = uploadInfo.Staging.buffer;
		copyInfo.dstBuffer = uploadInfo.Dest.buffer;
		copyInfo.regionCount = 1;
		copyInfo.pRegions = &bufferCopy;

		// Perform copy and save upload resource till EndUploadRegion is called
		vkCmdCopyBuffer2(copyList.GetBuffer(), &copyInfo);
		Table::Append<COPY_LIST_GROW_SIZE>(copyResInfo, copyResList,
			UploadInfo{ gfxQueueIndex, true, uploadInfo.Dest.buffer, uploadInfo.DestStage, uploadInfo.DestAccess, uploadInfo.Staging.buffer, stagingAlloc });
	}

	void Device::UploadBindTexture(UploadInfoTexture& uploadInfo)
	{
		ZE_ASSERT(copyResList != nullptr, "Empty upload list!");
	}

	void Device::UpdateBuffer(UploadInfoBufferUpdate& updateInfo)
	{
		ZE_VK_ENABLE_ID();
		ZE_ASSERT(copyResList != nullptr, "Empty upload list!");
		ZE_ASSERT(updateInfo.Data != nullptr, "Empty initial data!");

		// Ignore previous content and perform barrier for given resource
		VkBufferMemoryBarrier2 barrier = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2, nullptr };
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
		barrier.srcAccessMask = VK_ACCESS_2_NONE;
		barrier.dstStageMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		barrier.srcQueueFamilyIndex = updateInfo.LastUsedQueue;
		barrier.dstQueueFamilyIndex = gfxQueueIndex;
		barrier.buffer = updateInfo.Buffer;
		barrier.offset = 0;
		barrier.size = VK_WHOLE_SIZE;

		VkDependencyInfo depInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO, nullptr };
		depInfo.dependencyFlags = 0;
		depInfo.memoryBarrierCount = 0;
		depInfo.pMemoryBarriers = nullptr;
		depInfo.bufferMemoryBarrierCount = 1;
		depInfo.pBufferMemoryBarriers = &barrier;
		depInfo.imageMemoryBarrierCount = 0;
		depInfo.pImageMemoryBarriers = nullptr;
		vkCmdPipelineBarrier2(copyList.GetBuffer(), &depInfo);

		// Create staging buffer
		VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr };
		stagingBufferInfo.flags = 0;
		stagingBufferInfo.size = updateInfo.Bytes;
		stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		stagingBufferInfo.queueFamilyIndexCount = 0;
		stagingBufferInfo.pQueueFamilyIndices = nullptr;

		const U32 deviceIndex = 0;
		VkBindBufferMemoryDeviceGroupInfo deviceGroupInfo = { VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO, nullptr };
		deviceGroupInfo.deviceIndexCount = 1;
		deviceGroupInfo.pDeviceIndices = &deviceIndex;

		VkBindBufferMemoryInfo stagingBindInfo = { VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO, &deviceGroupInfo };
		ZE_VK_THROW_NOSUCC(vkCreateBuffer(device, &stagingBufferInfo, nullptr, &stagingBindInfo.buffer));
		ZE_VK_SET_ID(device, stagingBindInfo.buffer, VK_OBJECT_TYPE_BUFFER,
			"Staging update buffer [size:" + std::to_string(updateInfo.Bytes) + "]");

		Allocation stagingAlloc = allocator.AllocBuffer(*this, stagingBindInfo.buffer, Allocation::Usage::StagingToGPU);
		U8* mappedMemory = nullptr;
		allocator.GetAllocInfo(stagingAlloc, stagingBindInfo.memoryOffset, stagingBindInfo.memory, &mappedMemory);
		ZE_VK_THROW_NOSUCC(vkBindBufferMemory2(device, 1, &stagingBindInfo));

		// Copy data to staging buffer
		ZE_ASSERT(mappedMemory != nullptr, "Staging buffer always should be accessible from CPU!");
		memcpy(mappedMemory, updateInfo.Data, updateInfo.Bytes);

		VkBufferCopy2 bufferCopy = { VK_STRUCTURE_TYPE_BUFFER_COPY_2, nullptr };
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;
		bufferCopy.size = updateInfo.Bytes;

		VkCopyBufferInfo2 copyInfo = { VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2, nullptr };
		copyInfo.srcBuffer = stagingBindInfo.buffer;
		copyInfo.dstBuffer = updateInfo.Buffer;
		copyInfo.regionCount = 1;
		copyInfo.pRegions = &bufferCopy;

		// Perform copy and save upload resource till EndUploadRegion is called
		vkCmdCopyBuffer2(copyList.GetBuffer(), &copyInfo);
		Table::Append<COPY_LIST_GROW_SIZE>(copyResInfo, copyResList,
			UploadInfo{ barrier.srcQueueFamilyIndex, true, updateInfo.Buffer, updateInfo.DestStage, updateInfo.DestAccess, stagingBindInfo.buffer, stagingAlloc });
	}
}