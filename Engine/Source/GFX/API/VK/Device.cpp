#include "GFX/API/VK/Device.h"
#include "GFX/API/VK/VulkanException.h"
#include "GFX/CommandList.h"

// List of current required instance extensions
#define ZE_VK_REQUIRED_INSTANCE_EXT \
	X(VK_KHR_SURFACE_EXTENSION_NAME) \
	X(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)

// List of current required device extensions
#define ZE_VK_REQUIRED_DEVICE_EXT \
	X(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)

namespace ZE::GFX::API::VK
{
#if _ZE_DEBUG_GFX_API
	VkBool32 VKAPI_PTR Device::DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		Logger::InfoNoFile(pCallbackData->pMessage);
		return VK_FALSE;
	}
#endif

	U16 Device::GetExtensionIndexDynamic(const char* extName) noexcept
	{
		// __COUNTER__ is supported on MSVC, GCC and Clang, on other compilers should find alternative
		constexpr U16 COUNTER_BASE = __COUNTER__ + 1;

#define X(ext) if (std::strcmp(ext, extName) == 0) return __COUNTER__ - COUNTER_BASE; else
		ZE_VK_EXT_LIST
#undef X
			return KNOWN_EXTENSION_COUNT;
	}

	bool Device::FindQueueIndices(VkPhysicalDevice device, QueueFamilyIndices& indices) noexcept
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
					if (flags & VK_QUEUE_GRAPHICS_BIT)
					{
						supportedQueues[0] = true;
						indices.Gfx = i;
					}
					else
					{
						supportedQueues[1] = true;
						indices.Compute = i;
					}
				}
				else
				{
					supportedQueues[2] = true;
					indices.Copy = i;
				}
			}
			if (supportedQueues.all())
				break;
		}
		return supportedQueues.all();
	}

	void Device::InitVolk()
	{
		// Custom way of initializing Vulkan in order to correctly unload library
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
#if _ZE_PLATFORM_WINDOWS
		HMODULE lib = LoadLibrary("vulkan-1.dll");
		if (!lib)
			throw ZE_CMP_EXCEPT("Vulkan library missing! Cannot find [vulkan-1.dll]!");

		vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(lib, "vkGetInstanceProcAddr"));
#else
		void* lib = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
		if (!lib)
			lib = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
		if (!lib)
			throw ZE_CMP_EXCEPT("Vulkan library missing! Cannot find [libvulkan.so.1] or [libvulkan.so]!");

		vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(lib, "vkGetInstanceProcAddr"));
#endif
		vulkanLibModule = static_cast<LibraryHandle>(lib);
		volkInitializeCustom(vkGetInstanceProcAddr);
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
		std::vector<VkExtensionProperties> supportedExtensions(count);
		ZE_VK_THROW_NOSUCC(vkEnumerateInstanceExtensionProperties(nullptr, &count, supportedExtensions.data()));

#define X(ext) ext,
		std::vector<const char*> enabledExtensions
		{
#if _ZE_DEBUG_GFX_API
			VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
#if !_ZE_PLATFORM_LINUX
			ZE_VK_EXT_LIST_INSTANCE_PLATFORM
#endif
			ZE_VK_REQUIRED_INSTANCE_EXT
		};
#undef X

#define X(ext) GetExtensionIndex(ext),
		std::vector<U16> enabledExtensionsIndices
		{
#if _ZE_DEBUG_GFX_API
			GetExtensionIndex(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME),
			GetExtensionIndex(VK_EXT_DEBUG_UTILS_EXTENSION_NAME),
#endif
#if !_ZE_PLATFORM_LINUX
			ZE_VK_EXT_LIST_INSTANCE_PLATFORM
#endif
			ZE_VK_REQUIRED_INSTANCE_EXT
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
			extensionSupport[enabledExtensionsIndices.at(i)] = true;
		}
		enabledExtensionsIndices.clear();
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
		VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		instanceInfo.flags = 0;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledExtensionCount = static_cast<U32>(enabledExtensions.size());
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

	Device::QueueFamilyIndices Device::FindPhysicalDevice(VkPhysicalDeviceFeatures2& features)
	{
		ZE_VK_ENABLE();

		// Get present GPUs available in system
		U32 deviceCount = 0;
		ZE_VK_THROW_NOSUCC(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
		if (deviceCount == 0)
			throw ZE_CMP_EXCEPT("Current system have no avaiable GPUs!");

		QueueFamilyIndices queueIndices;
		if (deviceCount == 1)
		{
			// Only 1 GPU present so can skip all the hassle
			ZE_VK_THROW_NOSUCC(vkEnumeratePhysicalDevices(instance, &deviceCount, &physicalDevice));

			// Check if this GPU is enough for using in rendering
			vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
			if (!features.features.geometryShader || !FindQueueIndices(physicalDevice, queueIndices))
			{
				VkPhysicalDeviceProperties2 properties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, nullptr };
				vkGetPhysicalDeviceProperties2(physicalDevice, &properties);
				throw ZE_CMP_EXCEPT("GPU [" + std::string(properties.properties.deviceName) + "] don't support all of the graphics, compute and copy queues!");
			}
		}
		else
		{
			std::vector<VkPhysicalDevice> devices(deviceCount);
			ZE_VK_THROW_NOSUCC(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

			// Get most suited GPU for rendering using some ranking algorithm
			std::multimap<U16, std::pair<VkPhysicalDevice, QueueFamilyIndices>> deviceRank;
			for (VkPhysicalDevice dev : devices)
			{
				// Skip GPUs with not sufficient features
				vkGetPhysicalDeviceFeatures2(dev, &features);
				if (!features.features.geometryShader)
					continue;

				// Check if support 3 distinct queues
				if (!FindQueueIndices(dev, queueIndices))
					continue;

				U16 rank = 0;
				VkPhysicalDeviceProperties2 properties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, nullptr };
				vkGetPhysicalDeviceProperties2(dev, &properties);
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
				rank += properties.properties.limits.bufferImageGranularity / 100;
				// Smaller coherent memory chunks are better for aligning of mappable memory (HOST_VISIBLE without HOST_COHERENT)
				rank += properties.properties.limits.nonCoherentAtomSize / 50;

				// Handle GPU memory as one of the main indicators of performance
				VkPhysicalDeviceMemoryProperties2 memoryProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2, nullptr };
				vkGetPhysicalDeviceMemoryProperties2(dev, &memoryProps);
				for (U32 j = 0; j < memoryProps.memoryProperties.memoryHeapCount; ++j)
				{
					if (memoryProps.memoryProperties.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
						rank += Math::DivideRoundUp(memoryProps.memoryProperties.memoryHeaps[j].size, Math::GIGABYTE) * 100;
				}
				deviceRank.emplace(rank, std::make_pair(dev, queueIndices));
			}
			devices.clear();

			// Sanity check whether we have enough GPUs to choose from
			if (deviceRank.size() == 0)
				throw ZE_CMP_EXCEPT("None of the GPUs support geometry shaders and all of the graphics, compute and copy queues!");

			// Get one with highest score
			physicalDevice = deviceRank.end()->second.first;
			queueIndices = deviceRank.end()->second.second;
			vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
		}
		return queueIndices;
	}

	Device::Device(U32 descriptorCount, U32 scratchDescriptorCount)
	{
		ZE_VK_ENABLE_ID();
		InitVolk();
		CreateInstance();

		VkPhysicalDeviceVulkan13Features features3 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, nullptr };
		VkPhysicalDeviceVulkan12Features features2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, &features3 };
		VkPhysicalDeviceVulkan11Features features1 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, &features2 };
		VkPhysicalDeviceFeatures2 features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &features1 };
		QueueFamilyIndices queueIndices = FindPhysicalDevice(features);

		// Describe used queues
		const float queuePriority = 1.0f;
		const VkDeviceQueueCreateInfo queueInfos[3]
		{
			{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, queueIndices.Gfx, 1, &queuePriority },
			{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, queueIndices.Compute, 1, &queuePriority },
			{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, queueIndices.Copy, 1, &queuePriority }
		};

		// Check for device extensions
		U32 count = 0;
		ZE_VK_THROW_NOSUCC(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr));
		std::vector<VkExtensionProperties> supportedExtensions(count);
		ZE_VK_THROW_NOSUCC(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, supportedExtensions.data()));

#define X(ext) ext,
		std::vector<const char*> enabledExtensions{ ZE_VK_REQUIRED_DEVICE_EXT };
#undef X

#define X(ext) GetExtensionIndex(ext),
		std::vector<U16> enabledExtensionsIndices{ ZE_VK_REQUIRED_DEVICE_EXT };
#undef X

		auto isExtSupported = [&supportedExtensions](const char* name) -> bool
		{
			for (const VkExtensionProperties& ext : supportedExtensions)
				if (strcmp(ext.extensionName, name) == 0)
					return true;
			return false;
		};
		for (U32 i = 0; i < enabledExtensions.size(); ++i)
		{
			if (!isExtSupported(enabledExtensions.at(i)))
				throw ZE_CMP_EXCEPT("Required device extension is not supported: " + std::string(enabledExtensions.at(i)));
			extensionSupport[enabledExtensionsIndices.at(i)] = true;
		}
		enabledExtensionsIndices.clear();
		supportedExtensions.clear();

		// Create logic device
		VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr };
		deviceInfo.flags = 0;
		deviceInfo.queueCreateInfoCount = sizeof(queueInfos) / sizeof(VkDeviceQueueCreateInfo);
		deviceInfo.pQueueCreateInfos = queueInfos;
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
		deviceInfo.enabledExtensionCount = static_cast<U32>(enabledExtensions.size());
		deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
		deviceInfo.pEnabledFeatures = &features.features;

		ZE_VK_THROW_NOSUCC(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device));
		volkLoadDevice(device);
		vkGetDeviceQueue(device, queueIndices.Gfx, 0, &gfxQueue);
		vkGetDeviceQueue(device, queueIndices.Compute, 0, &computeQueue);
		vkGetDeviceQueue(device, queueIndices.Copy, 0, &copyQueue);
	}

	Device::~Device()
	{
		if (device)
			vkDestroyDevice(device, nullptr);
		if (instance)
			vkDestroyInstance(instance, nullptr);
#if _ZE_PLATFORM_WINDOWS
		if (vulkanLibModule)
		{
			const BOOL status = FreeLibrary(static_cast<HMODULE>(vulkanLibModule));
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

	void Device::Execute(GFX::CommandList* cls, U32 count) noexcept(!_ZE_DEBUG_GFX_API)
	{
	}
}