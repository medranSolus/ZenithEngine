#include "GFX/API/VK/Device.h"
#include "GFX/API/VK/VulkanException.h"
#include "GFX/CommandList.h"

namespace ZE::GFX::API::VK
{
#ifdef _ZE_MODE_DEBUG
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

	Device::Device(U32 descriptorCount, U32 scratchDescriptorCount)
	{
		ZE_VK_ENABLE_ID();
		ZE_VK_THROW_NOSUCC(volkInitialize());

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
		U32 extensionCount = 0;
		ZE_VK_THROW_NOSUCC(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
		VkExtensionProperties* supportedExtensions = new VkExtensionProperties[extensionCount];
		ZE_VK_THROW_NOSUCC(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions));

		std::vector<const char*> enabledExtensions
		{
#ifdef _ZE_MODE_DEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
			VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _ZE_PLATFORM_WINDOWS
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif !defined(_ZE_PLATFORM_LINUX)
#	error Building for platform not supported by current Vulkan implementation!
#endif
		};
		std::vector<U16> enabledExtensionsIndices
		{
#ifdef _ZE_MODE_DEBUG
			GetExtensionIndex(VK_EXT_DEBUG_UTILS_EXTENSION_NAME),
#endif
			GetExtensionIndex(VK_KHR_SURFACE_EXTENSION_NAME),
#ifdef _ZE_PLATFORM_WINDOWS
			GetExtensionIndex(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)
#endif
		};

		auto isExtSupported = [&extensionCount, &supportedExtensions](const char* name) -> bool
		{
			for (U32 i = 0; i < extensionCount; ++i)
				if (strcmp(supportedExtensions[i].extensionName, name) == 0)
					return true;
			return false;
		};
		for (U32 i = 0; const char* name : enabledExtensions)
		{
			if (!isExtSupported(name))
				throw ZE_CMP_EXCEPT("Required instance extension is not supported: " + std::string(name));
			extensionSupport[enabledExtensionsIndices.at(i++)] = true;
		}
		enabledExtensionsIndices.clear();

#ifdef _ZE_PLATFORM_LINUX
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
		instanceInfo.enabledLayerCount = 0;
		instanceInfo.ppEnabledLayerNames = nullptr; // VK_LAYER_KHRONOS_validation VK_LAYER_KHRONOS_profiles VK_LAYER_KHRONOS_synchronization2
		instanceInfo.enabledExtensionCount = static_cast<U32>(enabledExtensions.size());
		instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();

		// Specify features for type of build
		VkValidationFeaturesEXT validationFeatures = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, nullptr };

#ifdef _ZE_MODE_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugMessenger = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, nullptr };
		debugMessenger.flags = 0;
		debugMessenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugMessenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		// if VK_EXT_device_address_binding_report then VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT
		debugMessenger.pfnUserCallback = DebugMessengerCallback;
		debugMessenger.pUserData = nullptr;
		instanceInfo.pNext = &debugMessenger;

		const VkValidationFeatureEnableEXT enabledValidations[] =
		{
#ifdef _ZE_DEBUG_GPU_VALIDATION
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
#else
		validationFeatures.enabledValidationFeatureCount = 0;
		validationFeatures.pEnabledValidationFeatures = nullptr;
#endif

#ifdef _ZE_MODE_RELEASE
		const VkValidationFeatureDisableEXT disabledFeature = VK_VALIDATION_FEATURE_DISABLE_ALL_EXT;
		validationFeatures.disabledValidationFeatureCount = 1;
		validationFeatures.pDisabledValidationFeatures = &disabledFeature;
#else
		validationFeatures.disabledValidationFeatureCount = 0;
		validationFeatures.pDisabledValidationFeatures = nullptr;
#endif

		// Set validation options if supported
		if (isExtSupported(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME))
		{
			enabledExtensions.emplace_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
			extensionSupport[GetExtensionIndex(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)] = true;

#ifdef _ZE_MODE_DEBUG
			debugMessenger.pNext = &validationFeatures;
#else
			instanceInfo.pNext = &validationFeatures;
#endif
		}
		delete[] supportedExtensions;

		// Create instance
		ZE_VK_THROW_NOSUCC(vkCreateInstance(&instanceInfo, nullptr, &instance));
		volkLoadInstanceOnly(instance);

		//volkLoadDevice(nullptr);
	}

	Device::~Device()
	{
		vkDestroyInstance(instance, nullptr);
	}

	void Device::Execute(GFX::CommandList* cls, U32 count) noexcept(ZE_NO_DEBUG)
	{
	}
}