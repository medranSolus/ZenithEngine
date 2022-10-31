#pragma once
// Headers needed for Vulkan
#include "WarningGuardOn.h"
#include "volk.h"
#include "WarningGuardOff.h"

/* Definitions of extensions
*
* Every listed extension here have tags attached to it, which will help
* in searching for possible use case for that extension.
* Possible extension tags:
*
* EXT_DEBUG  - debugging and other tools
* EXT_RT     - ray tracing
* EXT_WINDOW - windowning and surfaces
* EXT_SHADER - shader related operations
* EXT_MEMORY - memory managment
* EXT_EXMEM  - memory sharing (memory from external sources)
* EXT_QUERY  - queries and info
* EXT_PERF   - performance and optimizations
* EXT_FEAT   - general features
*
* Additionally to indicate if any of the extensions were promoted to core Vulkan,
* there are tags indicating at which version it happened: 1.1, 1.2 or 1.3
*/
#pragma region Instance extensions
// List of platform independent instance extension names, intended for use in X() macro
#define ZE_VK_EXT_LIST_INSTANCE_GENERAL \
	X(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)                      /* Debug instrumentation [EXT_DEBUG] */ \
	X(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME)            /* Multiple physical devices as one logical [EXT_FEAT] [1.1] */ \
	X(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME)              /* Full control over display [EXT_WINDOW] */ \
	X(VK_KHR_DISPLAY_EXTENSION_NAME)                          /* Display handling [EXT_WINDOW] */ \
	X(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME)          /* Quering for display vertical blank [EXT_WINDOW] [EXT_QUERY] */ \
	X(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME)      /* Info about imported external fences [EXT_EXMEM] [EXT_QUERY] [1.1] */ \
	X(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME)     /* Info about imported external memory regions [EXT_EXMEM] [EXT_QUERY] [1.1] */ \
	X(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME)  /* Info about imported external semaphores [EXT_EXMEM] [EXT_QUERY] [1.1] */ \
	X(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME)         /* Informations about displays [EXT_WINDOW] [EXT_QUERY] */ \
	X(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) /* Informations about GPUs [EXT_QUERY] */ \
	X(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)       /* Informations about surfaces [EXT_WINDOW] [EXT_QUERY] */ \
	X(VK_KHR_SURFACE_EXTENSION_NAME)                          /* Window surface management [EXT_WINDOW] */ \
	X(VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME)   /* Info about protected session for surface [EXT_FEAT] [EXT_QUERY] */ \
	X(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)            /* Additional colorspaces for swapchain surface (HDR) [EXT_WINDOW] [EXT_FEAT] */ \
	X(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)              /* Specifying validation checks [EXT_DEBUG] */

// Platform specific extensions (WSI)
#ifdef _ZE_PLATFORM_WINDOWS
// List of platform dependent instance extension names, intended for use in X() macro
#	define ZE_VK_EXT_LIST_INSTANCE_PLATFORM \
		X(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)         /* Surface for Windows targets [EXT_WINDOW] */
#elif defined(_ZE_PLATFORM_LINUX)
// List of platform dependent instance extension names, intended for use in X() macro
#	define ZE_VK_EXT_LIST_INSTANCE_PLATFORM \
		X(VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME)   /* Full control over Linux DRM display [EXT_WINDOW] */ \
		X(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME)  /* Full control over Linux Xlib display [EXT_WINDOW] */ \
		X(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME)      /* Surface for DirectFB Linux targets [EXT_WINDOW] */ \
		X(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME)       /* Surface for Wayland Linux targets [EXT_WINDOW] */ \
		X(VK_KHR_XCB_SURFACE_EXTENSION_NAME)           /* Surface for Xcb Linux targets [EXT_WINDOW] */ \
		X(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)          /* Surface for Xlib Linux targets [EXT_WINDOW] */
#elif defined(_ZE_PLATFORM_ANDROID)
#	define ZE_VK_EXT_LIST_INSTANCE_PLATFORM \
		X(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)       /* Surface for android targets [EXT_WINDOW] */
#elif defined(_ZE_PLATFORM_FUCHSIA)
#	define ZE_VK_EXT_LIST_INSTANCE_PLATFORM \
		X(VK_FUCHSIA_IMAGEPIPE_SURFACE_EXTENSION_NAME) /* Surface for Fuchsia OS targets [EXT_WINDOW] */
#elif defined(_ZE_PLATFORM_NSWITCH)
#	define ZE_VK_EXT_LIST_INSTANCE_PLATFORM \
		X(VK_NN_VI_SURFACE_EXTENSION_NAME)             /* Surface for Nintendo Switch targets [EXT_WINDOW] */
#else
#	error Vulkan not supported for that platform!
#endif

// List of instance extension names, intended for use in X() macro
#define ZE_VK_EXT_LIST_INSTANCE ZE_VK_EXT_LIST_INSTANCE_GENERAL ZE_VK_EXT_LIST_INSTANCE_PLATFORM
#pragma endregion

// Workaround, will be in Vulkan SDK 1.3.230+
#ifndef VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME
#	define VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME "VK_EXT_opacity_micromap"
#else
#	error Vulkan SDK updated, you can delete this section now!
#endif

// List of device extension names, intended for use in X() macro
#define ZE_VK_EXT_LIST_DEVICE \
	X(VK_EXT_VALIDATION_CACHE_EXTENSION_NAME) /* Caching validation checks for multiple runs [EXT_DEBUG] */ \
	X(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) /* Managing acceleration structures for RT [EXT_RT] */ \
	X(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) /* Managing RT pipelines [EXT_RT] */ \
	X(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME) /* Minor RT tweaks [EXT_RT] */ \
	X(VK_KHR_RAY_QUERY_EXTENSION_NAME) /* Inline RT (DXR 1.1) [EXT_RT] */ \
	X(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) /* Async CPU acceleration structures builds [EXT_RT] */ \
	X(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME) /* Handling opaque and transparent geometry (RTX 30+) [EXT_RT] */ \
	X(VK_NV_RAY_TRACING_MOTION_BLUR_EXTENSION_NAME) /* Faster tracing of geometry in motion [EXT_RT] */

// List of extension names, intended for use in X() macro
#define ZE_VK_EXT_LIST ZE_VK_EXT_LIST_INSTANCE ZE_VK_EXT_LIST_DEVICE