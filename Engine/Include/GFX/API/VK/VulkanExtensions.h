#pragma once
// Headers needed for Vulkan
#include "WarningGuardOn.h"
#include "volk.h"
#include "WarningGuardOff.h"

/* Definitions of extensions (up to v1.3.231)
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
* EXT_VIDEO  - video processing
*
* Additionally to indicate if any of the extensions were promoted to core Vulkan,
* there are tags indicating at which version it happened: 1.1, 1.2 or 1.3
*/
#pragma region Instance extensions
// Platform specific extensions (WSI)
#if _ZE_PLATFORM_WINDOWS
// List of required platform dependent instance extension names, intended for use in X() macro
#	define ZE_VK_EXT_LIST_INSTANCE_PLATFORM_REQUIRED \
	X(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)                    /* Surface for Windows targets [EXT_WINDOW] */
#elif _ZE_PLATFORM_LINUX
// List of required platform dependent instance extension names, intended for use in X() macro
#	define ZE_VK_EXT_LIST_INSTANCE_PLATFORM_REQUIRED \
	X(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME)                  /* Surface for Wayland Linux targets [EXT_WINDOW] */ \
	X(VK_KHR_XCB_SURFACE_EXTENSION_NAME)                      /* Surface for Xcb Linux targets [EXT_WINDOW] */ \
	X(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)                     /* Surface for Xlib Linux targets [EXT_WINDOW] */ \
	X(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME)             /* Full control over Linux Xlib display [EXT_WINDOW] */ \
	X(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME)                 /* Surface for DirectFB Linux targets [EXT_WINDOW] */
#elif _ZE_PLATFORM_ANDROID
// List of required platform dependent instance extension names, intended for use in X() macro
#	define ZE_VK_EXT_LIST_INSTANCE_PLATFORM_REQUIRED \
	X(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)                  /* Surface for android targets [EXT_WINDOW] */
#elif _ZE_PLATFORM_FUCHSIA
// List of required platform dependent instance extension names, intended for use in X() macro
#	define ZE_VK_EXT_LIST_INSTANCE_PLATFORM_REQUIRED \
	X(VK_FUCHSIA_IMAGEPIPE_SURFACE_EXTENSION_NAME)            /* Surface for Fuchsia OS targets [EXT_WINDOW] */
#elif _ZE_PLATFORM_NSWITCH
// List of required platform dependent instance extension names, intended for use in X() macro
#	define ZE_VK_EXT_LIST_INSTANCE_PLATFORM_REQUIRED \
	X(VK_NN_VI_SURFACE_EXTENSION_NAME)                        /* Surface for Nintendo Switch targets [EXT_WINDOW] */
#else
#	error Vulkan not supported for that platform!
#endif

// List of required platform independent instance extension names, intended for use in X() macro
#define ZE_VK_EXT_LIST_INSTANCE_REQUIRED \
	X(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) /* Informations about GPUs [EXT_QUERY] */ \
	X(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)       /* Informations about surfaces [EXT_WINDOW] [EXT_QUERY] */ \
	X(VK_KHR_SURFACE_EXTENSION_NAME)                          /* Window surface management [EXT_WINDOW] */

// List of platform independent instance extension names, used in debug targets, intended for use in X() macro
#define ZE_VK_EXT_LIST_INSTANCE_DEBUG \
	X(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)              /* Specifying validation checks (must be first in this list!) [EXT_DEBUG] */ \
	X(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)                      /* Debug instrumentation [EXT_DEBUG] */

// List of optional platform independent instance extension names, intended for use in X() macro
#define ZE_VK_EXT_LIST_INSTANCE_OPTIONAL \
	X(VK_KHR_DISPLAY_EXTENSION_NAME)                          /* Display handling [EXT_WINDOW] */ \
	X(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME)      /* Info about imported external fences [EXT_EXMEM] [EXT_QUERY] [1.1] */ \
	X(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME)     /* Info about imported external memory regions [EXT_EXMEM] [EXT_QUERY] [1.1] */ \
	X(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME)  /* Info about imported external semaphores [EXT_EXMEM] [EXT_QUERY] [1.1] */ \
	X(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME)              /* Full control over display [EXT_WINDOW] */

// List of currently not used platform independent instance extension names, intended for use in X() macro
#define ZE_VK_EXT_LIST_INSTANCE_NOT_USED \
	X(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME)            /* Multiple physical devices as one logical [EXT_FEAT] [1.1] */ \
	X(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME)         /* Informations about displays [EXT_WINDOW] [EXT_QUERY] */ \
	X(VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME)   /* Info about protected session for surface [EXT_FEAT] [EXT_QUERY] */ \
	X(VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME)              /* Full control over Linux DRM display [EXT_WINDOW] */ \
	X(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME)          /* Quering for display vertical blank [EXT_WINDOW] [EXT_QUERY] */ \
	X(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME)                 /* Creation of surface not used by any windowing system for testing [EXT_WINDOW] [EXT_DEBUG] */ \
	X(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)            /* Additional colorspaces for swapchain surface (HDR) [EXT_WINDOW] [EXT_FEAT] */ \
	X(VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME)             /* Don't require surface with single presentation target [EXT_WINDOW] [EXT_FEAT] */

// List of instance extension names, intended for use in X() macro
#define ZE_VK_EXT_LIST_INSTANCE ZE_VK_EXT_LIST_INSTANCE_PLATFORM_REQUIRED ZE_VK_EXT_LIST_INSTANCE_REQUIRED ZE_VK_EXT_LIST_INSTANCE_DEBUG ZE_VK_EXT_LIST_INSTANCE_OPTIONAL
#pragma endregion

#pragma region Device extensions
// To be changed with new SDKs
#ifndef VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME
#	define VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME "VK_EXT_descriptor_buffer"
#else
#	error Extension already supported!
#endif
#ifndef VK_NV_COPY_MEMORY_INDIRECT_EXTENSION_NAME
#	define VK_NV_COPY_MEMORY_INDIRECT_EXTENSION_NAME "VK_NV_copy_memory_indirect"
#else
#	error Extension already supported!
#endif
#ifndef VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME
#	define VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME "VK_NV_memory_decompression"
#else
#	error Extension already supported!
#endif
#ifndef VK_NV_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME
#	define VK_NV_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME "VK_NV_ray_tracing_invocation_reorder"
#else
#	error Extension already supported!
#endif

// List of device extension names, required by the engine, intended for use in X() macro
#define ZE_VK_EXT_LIST_DEVICE_REQUIRED \
	X(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME)                                /* Bind multiple memory regions and allows aliasing [EXT_MEMORY] [1.1] */ \
	X(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)                          /* Better way for creating render passes [EXT_FEAT] [1.2] */ \
	X(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)                         /* Dedicated allocs for certain resources [EXT_MEMORY] [EXT_PERF] [1.1] */ \
	X(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)                        /* Automatic resolve for multisampled depth stencil [EXT_PERF] [EXT_FEAT] [1.2] */ \
	X(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)                            /* Normal rendering without render passes [EXT_FEAT] [1.3] */ \
	X(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME)                    /* Better query for memory info [EXT_MEMORY] [EXT_QUERY] [1.1] */ \
	X(VK_KHR_MAINTENANCE_1_EXTENSION_NAME)                                /* Minor tweaks overlooked in original release [EXT_FEAT] [1.1] */ \
	X(VK_KHR_MAINTENANCE_2_EXTENSION_NAME)                                /* Minor tweaks overlooked in original release [EXT_FEAT] [1.1] */ \
	X(VK_KHR_MULTIVIEW_EXTENSION_NAME)                                    /* Render same things but with different view, useful for VR [EXT_FEAT] [EXT_PERF] [1.1] */ \
	X(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)                     /* Support for YCbCr textures [EXT_FEAT] [1.1] */ \
	X(VK_KHR_SWAPCHAIN_EXTENSION_NAME)                                    /* Creation of swapchain [EXT_WINDOW] */ \
	X(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)                            /* New better interface for managing synchronization with GPU [EXT_FEAT] [1.3] */ \
	X(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)                           /* Copy of D3D12 fence behavior for semaphores [EXT_FEAT] [1.2] */ \
	X(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME)                            /* Explicit control over depth clip (like in D3D) [EXT_FEAT] */ \
	X(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)                       /* Dynamic states for stencil operations, viewports, scissors, depth and primitives [EXT_FEAT] [EXT_PERF] [1.3] */ \
	X(VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME)              /* Use value in index buffers to restart primitive [EXT_FEAT] */ \
	X(VK_EXT_4444_FORMATS_EXTENSION_NAME)                                 /* New 16 bit packed pixel formats [EXT_FEAT] [1.3] */

// List of device extension names, used in debug targets, intended for use in X() macro
#define ZE_VK_EXT_LIST_DEVICE_DEBUG

// Platform dependent optional extensions
#if _ZE_PLATFORM_WINDOWS
// List of optional platform dependent device extension names, intended for use in X() macro
#	define ZE_VK_EXT_LIST_DEVICE_PLATFORM_OPTIONAL \
	X(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME)                         /* Get Windows fence from Vulkan [EXT_EXMEM] */ \
	X(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME)                        /* Get Windows allocated memory from Vulkan [EXT_EXMEM] */ \
	X(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME)                     /* Get Windows fence from Vulkan [EXT_EXMEM] */
#elif _ZE_PLATFORM_ANDROID
// List of optional platform dependent device extension names, intended for use in X() macro
#	define ZE_VK_EXT_LIST_DEVICE_PLATFORM_OPTIONAL \
	X(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)  /* Importing/exporting Android native buffers [EXT_EXMEM] */
#elif _ZE_PLATFORM_FUCHSIA
// List of optional platform dependent device extension names, intended for use in X() macro
#	define ZE_VK_EXT_LIST_DEVICE_PLATFORM_OPTIONAL \
	X(VK_FUCHSIA_BUFFER_COLLECTION_EXTENSION_NAME)                        /* Sharing similar purpose buffers on Fuchsia OS for better compatibility with system conventions [EXT_EXMEM] */ \
	X(VK_FUCHSIA_EXTERNAL_MEMORY_EXTENSION_NAME)                          /* Import or export memory from external sources on Fuchsia OS [EXT_EXMEM] */ \
	X(VK_FUCHSIA_EXTERNAL_SEMAPHORE_EXTENSION_NAME)                       /* Import or export semaphore from external sources on Fuchsia OS [EXT_EXMEM] */
#endif

// List of optional device extension extension names, intended for use in X() macro
#define ZE_VK_EXT_LIST_DEVICE_OPTIONAL \
	X(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME)                               /* Access implemenation handle of internal fence [EXT_EXMEM] [1.1] */ \
	X(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME)                              /* Access implemenation handle of internal memory [EXT_EXMEM] [1.1] */ \
	X(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME)                           /* Access implemenation handle of internal semaphore [EXT_EXMEM] [1.1] */ \
	X(VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME)                         /* Less restrictive alignment restrictions [EXT_FEAT] [EXT_MEMORY] [1.1] */ \
	X(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME)                          /* More explicit memory handling [EXT_MEMORY] [1.2] */ \
	X(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME)                   /* Support fo conservative rasterization [EXT_FEAT] */ \
	X(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME)                         /* New queue for resource ownership transfer for external memory [EXT_EXMEM] [1.3] */ \
	X(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME)                             /* 8-bit indices [EXT_MEMORY] [EXT_FEAT] */ \
	X(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)                                /* Getting info about current usage of memory [EXT_MEMORY] [EXT_QUERY] */ \
	X(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME)                              /* Influencing what memory regions would be evicted first [EXT_MEMORY] */ \
	X(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME)                 /* Enable memory paging [EXT_MEMORY] [EXT_FEAT] */ \
	X(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME)                        /* Get better info about waves sizes and control them [EXT_QUERY] [EXT_FEAT] [1.3] */ \
	X(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME)                       /* A bit slower memory but useful for debug [EXT_MEMORY] [EXT_DEBUG] */ \
	X(VK_AMD_MEMORY_OVERALLOCATION_BEHAVIOR_EXTENSION_NAME)               /* Controlling behavior of oversubscription of memory [EXT_MEMORY] */ \
	X(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME)                     /* Using bigger depth stencils with smaller render targets in multisampled rendering (counterpart to VK_NV_framebuffer_mixed_samples) [EXT_FEAT] */ \
	X(VK_AMD_PIPELINE_COMPILER_CONTROL_EXTENSION_NAME)                    /* Control over PSO compilation process [EXT_PERF] */ \
	X(VK_AMD_RASTERIZATION_ORDER_EXTENSION_NAME)                          /* Change order of resterization process for better parallelism [EXT_PERF] */ \
	X(VK_NV_DEDICATED_ALLOCATION_IMAGE_ALIASING_EXTENSION_NAME)           /* Aliasing of memory in dedicated allocs [EXT_MEMORY] */ \
	X(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME)                     /* Using different sizes of depth and color buffer to output on smaller target, usable on path rendering (counterpart to VK_AMD_mixed_attachment_samples) [EXT_FEAT] */ \
	X(VK_NV_REPRESENTATIVE_FRAGMENT_TEST_EXTENSION_NAME)                  /* Faster early-z due to less stores [EXT_SHADER] [EXT_PERF] */ \
	ZE_VK_EXT_LIST_DEVICE_PLATFORM_OPTIONAL

// List of device extension names, currently not used by basic version of the engine, intended for use in X() macro
#define ZE_VK_EXT_LIST_DEVICE_NOT_USED \
	X(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)                       /* Managing acceleration structures for RT [EXT_RT] */ \
	X(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)                        /* Get GPU address of buffers for usage in shaders, RT or tools [EXT_FEAT] [EXT_SHADER] [EXT_MEMORY] [1.2] */ \
	X(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME)                              /* Better copy commands interfaces [EXT_FEAT] [1.3] */ \
	X(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)                     /* Async CPU acceleration structures builds [EXT_RT] [EXT_PERF] */ \
	X(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME)                   /* Faster updates of common descriptor sets [EXT_PERF] [1.1] */ \
	X(VK_KHR_DEVICE_GROUP_EXTENSION_NAME)                                 /* Multiple physical devices as one logical [EXT_FEAT] [1.1] */ \
	X(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME)                            /* Creation of swapchain directly from display [EXT_WINDOW] */ \ \
	X(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME)                          /* Get the number of commands used in indirect drawing from buffer (can be produced by GPU before) [EXT_FEAT] [EXT_PERF] [1.2] */ \ \
	X(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME)                            /* Get info what driver is used on GPU [EXT_QUERY] [1.2] */ \ \
	X(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME)                            /* Get Linux file descriptor for fence from Vulkan [EXT_EXMEM] */ \
	X(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME)                           /* Get Linux file descriptor for allocated memory from Vulkan [EXT_EXMEM] */ \
	X(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME)                        /* Get Linux file descriptor for fence from Vulkan [EXT_EXMEM] */ \
	X(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)                       /* Format feature flags are now 64 bit due to running out of space [1.3] */ \
	X(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME)                  /* New barycentric modes [EXT_SHADER] */ \
	X(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)                        /* Specifying how pixels should be shaded (VRS, LOD) [EXT_FEAT] */ \
	X(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME)                            /* Provide formats that mutable image will be used in [EXT_PERF] [1.2] */ \
	X(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME)                        /* Create framebuffer without previously created images [EXT_FEAT] [1.2] */ \
	X(VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME)                          /* Present only part of surface to screen [EXT_WINDOW] [EXT_PERF] */ \
	X(VK_KHR_GLOBAL_PRIORITY_EXTENSION_NAME)                              /* Setting queue priorities [EXT_FEAT] [EXT_PERF] */ \
	X(VK_KHR_MAINTENANCE_3_EXTENSION_NAME)                                /* Minor tweaks overlooked in original release regarding limits of descriptors and allocations [EXT_FEAT] [1.1] */ \
	X(VK_KHR_MAINTENANCE_4_EXTENSION_NAME)                                /* Collection of minor features too small for their own extensions [EXT_FEAT] [1.3] */ \
	X(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME)                            /* Query performance counter [EXT_QUERY] [EXT_PERF] */ \
	X(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME)               /* Query stats about compiled PSO [EXT_QUERY] [EXT_DEBUG] [EXT_PERF] */ \
	X(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME)                             /* Create set of shaders to be linked into other PSOs [EXT_PERF] [EXT_SHADER] */ \
	X(VK_KHR_PRESENT_ID_EXTENSION_NAME)                                   /* Aquire identification token for present operation [EXT_WINDOW] */ \
	X(VK_KHR_PRESENT_WAIT_EXTENSION_NAME)                                 /* Controlling when present operation is being held [EXT_WINDOW] */ \
	X(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)                              /* Something like descriptor inline binding in root signature in D3D12 [EXT_FEAT] [EXT_PERF] */ \
	X(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME)                    /* Minor RT tweaks [EXT_RT] */ \
	X(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)                         /* Managing RT pipelines [EXT_RT] */ \
	X(VK_KHR_RAY_QUERY_EXTENSION_NAME)                                    /* Inline RT (DXR 1.1) [EXT_RT] */ \
	X(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME)                 /* New sampler addressing mode (mirror clamp to edge) [EXT_FEAT] [1.2] */ \
	X(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME)               /* Separate layouts for depth and stencil buffers [EXT_FEAT] [1.2] */ \
	X(VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME)                          /* 64 bit integer atomics in shaders [EXT_SHADER] [EXT_FEAT] [1.2] */ \
	X(VK_KHR_SHADER_CLOCK_EXTENSION_NAME)                                 /* Access to clock in shaders [EXT_SHADER] */ \
	X(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME)                       /* New builtin data input for vertex shaders [EXT_SHADER] [1.1] */ \
	X(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME)                        /* Control float computation properties [EXT_SHADER] [1.2] */ \
	X(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME)                          /* Enabled 16 bit float and 8 bit integer arithmetics in shaders [EXT_SHADER] [EXT_FEAT] [EXT_PERF] [1.2] */ \
	X(VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME)                   /* Enabling integer dot product operation in shaders [EXT_SHADER] [EXT_FEAT] [1.3] */ \
	X(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)                     /* Non semantic instruction sets that can be defined in module (?) [EXT_SHADER] [EXT_FEAT] [1.3] */ \
	X(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME)               /* Allow most of the types for Non Uniform Group Operations [EXT_SHADER] [1.2] */ \
	X(VK_KHR_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_EXTENSION_NAME)         /* Better reconverging with uniform subgroups [EXT_SHADER] [EXT_PERF] */ \
	X(VK_KHR_SHADER_TERMINATE_INVOCATION_EXTENSION_NAME)                  /* Better instruction for discard when used with VK_EXT_shader_demote_to_helper_invocation [EXT_SHADER] [1.3] */ \
	X(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME)                     /* Using surface while it's being presented [EXT_WINDOW] */ \
	X(VK_KHR_SPIRV_1_4_EXTENSION_NAME)                                    /* Support for SPIR-V 1.4 [EXT_FEAT] [EXT_SHADER] [1.2] */ \
	X(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME)                 /* Storage decorator for buffers [EXT_SHADER] [1.1] */ \
	X(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME)                     /* Use swapchain with different compatible formats view [EXT_WINDOW] */ \
	X(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME)               /* Tighter alignment for arrays and structs in uniform buffers [EXT_SHADER] [1.2] */ \
	X(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME)                            /* New addressing modes in shaders [EXT_SHADER] [1.1] */ \
	X(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME)             /* Specifying layout of group shared memory (can handle aliasing based on lifecycle) [EXT_SHADER] */ \
	X(VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME)             /* Zero initialized workgroup memory [EXT_SHADER] [1.3] */ \
	X(VK_KHR_16BIT_STORAGE_EXTENSION_NAME)                                /* 16 bit types in input/outpus of the shaders [EXT_FEAT] [1.1] */ \
	X(VK_KHR_8BIT_STORAGE_EXTENSION_NAME)                                 /* 8 bit types in input/outpus of the shaders [EXT_FEAT] [1.2] */ \
	\
	X(VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME)                             /* Decode compressed textures to more compact representation [EXT_PERF] */ \
	X(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME)              /* Read from same resource that is being written in pixel shader (similar to UAV, sampler feedback?) [EXT_FEAT] */ \
	X(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME)                     /* More blend operations [EXT_FEAT] */ \
	X(VK_EXT_BORDER_COLOR_SWIZZLE_EXTENSION_NAME)                         /* Fixed custom border color with images without identity mapping (see VK_EXT_custom_border_color) [EXT_FEAT] */ \
	X(VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME)                        /* Timestamps from different sources [EXT_QUERY] [EXT_PERF] */ \
	X(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME)                           /* Controling writing to render targets via dynamic state [EXT_FEAT] [EXT_FEAT] */ \
	X(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME)                        /* Control on the fly whichc command will be executed based on memory values [EXT_FEAT] */ \
	X(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME)                          /* Use custom border color with samplers [EXT_FEAT] */ \
	X(VK_EXT_DEPTH_CLAMP_ZERO_ONE_EXTENSION_NAME)                         /* Ensure clamping depth in [0;1] range [EXT_FEAT] */ \
	X(VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME)                     /* Depth beyond [0;1] [EXT_FEAT] */ \
	X(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME)                            /* Shader-accessible descriptors managed directly in memory [EXT_MEMORY] [EXT_FEAT] */ \
	X(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)                          /* Non-uniform indexing of resources in shaders [EXT_SHADER] [EXT_FEAT] [1.2] */ \
	X(VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME)                /* Tracking memory pages used by resources [EXT_DEBUG] */ \
	X(VK_EXT_DEVICE_FAULT_EXTENSION_NAME)                                 /* Info about TDR [EXT_DEBUG] */ \
	X(VK_EXT_DEVICE_MEMORY_REPORT_EXTENSION_NAME)                         /* Callback for more memory usage info [EXT_DEBUG] */ \
	X(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME)                           /* Similar to scissors, discarding regions of render target [EXT_FEAT] [EXT_PERF] */ \
	X(VK_EXT_DISPLAY_CONTROL_EXTENSION_NAME)                              /* Some direct control over display [EXT_WINDOW] */ \
	X(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)                     /* Dynamic states for depth bias, patch control points, primitive restart and rasterizer discard [EXT_FEAT] [EXT_PERF] [1.3] */ \
	X(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME)                     /* Almost everything in PSO as dynamic state [EXT_FEAT] [EXT_PERF] */ \
	X(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME)                         /* Import memory directly from the pointer [EXT_EXMEM] */ \
	X(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME)                         /* Specify map of regions where pixel shader can be run fewer times (VRS) [EXT_SHADER] [EXT_FEAT] [EXT_PERF] */ \
	X(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME)                       /* Reduce latency and better behavior for VRS [EXT_FEAT] [EXT_PERF] */ \
	X(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME)                    /* Critical section in pixel shader [EXT_SHADER] */ \
	X(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME)                        /* Not sure if better to do fullscreen borderless as on D3D (extension only for Windows) [EXT_WINDOW] [EXT_FEAT] */ \
	X(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)                    /* Caching parts of compiled PSOs for later [EXT_PERF] */ \
	X(VK_EXT_HDR_METADATA_EXTENSION_NAME)                                 /* Using HDR with SMPTE 2086 and CTA 861.3 [EXT_WINDOW] [EXT_FEAT] */ \
	X(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)                             /* Reset queries from CPU side [EXT_QUERY] [EXT_FEAT] [1.2] */ \
	X(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME)                    /* Control compression of images (only one mobile supports it...) [EXT_PERF] */ \
	X(VK_EXT_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_EXTENSION_NAME)          /* Control compression of swapchain [EXT_WINDOW] */ \
	X(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME)                    /* Better handling specific Linux formats [EXT_WINDOW] */ \
	X(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME)                             /* Strict out of bound access behavior for images [EXT_DEBUG] [1.3] */ \
	X(VK_EXT_IMAGE_VIEW_MIN_LOD_EXTENSION_NAME)                           /* Specify minimal LOD [EXT_FEAT] */ \
	X(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME)                         /* Similar to push constants but can be reused [EXT_FEAT] [1.3] */ \
	X(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME)                           /* New options for line rendering [EXT_FEAT] */ \
	X(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME)                           /* No unnecessary synchronization on non-write resources [EXT_PERF] */ \
	X(VK_EXT_MESH_SHADER_EXTENSION_NAME)                                  /* Support for mesh shaders [EXT_FEAT] [EXT_SHADER] */ \
	X(VK_EXT_MULTI_DRAW_EXTENSION_NAME)                                   /* Faster multiple commands with same state [EXT_PERF] */ \
	X(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME)        /* Less memory consumed when using certain multisampled modes [EXT_PERF] */ \
	X(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME)                      /* Similar descriptor handling to D3D12 [EXT_FEAT] */ \
	X(VK_EXT_NON_SEAMLESS_CUBE_MAP_EXTENSION_NAME)                        /* Disable cubemap edge handling in sampling for compatibility with other APIs [EXT_FEAT] */ \
	X(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME)                             /* Handling opaque and transparent geometry (RTX 30+) [EXT_RT] [EXT_PERF] */ \
	X(VK_EXT_PCI_BUS_INFO_EXTENSION_NAME)                                 /* Get data about PCI bus of GPU [EXT_QUERY] */ \
	X(VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME)                          /* Get info about Linux DRM [EXT_QUERY] [EXT_WINDOW] */ \
	X(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME)              /* More deterministic time control for PSO creation [EXT_FEAT] [EXT_PERF] [1.3] */ \
	X(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME)                   /* Get info about built PSO for swapping some pipeline caches [EXT_FEAT] [EXT_QUERY] [1.3] */ \
	X(VK_EXT_PIPELINE_PROPERTIES_EXTENSION_NAME)                          /* Accessing PSO indentifier [EXT_FEAT] */ \
	X(VK_EXT_PIPELINE_PROTECTED_ACCESS_EXTENSION_NAME)                    /* Protected session enabled per pipeline, not per device [EXT_FEAT] */ \
	X(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME)                          /* Stricter access to resources in pipeline [EXT_DEBUG] */ \
	X(VK_EXT_POST_DEPTH_COVERAGE_EXTENSION_NAME)                          /* Control for pixel shader over coverage of early depth and stencil tests [EXT_SHADER] */ \
	X(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME)                   /* Get data about amount of processed primitives [EXT_QUERY] */ \
	X(VK_EXT_PRIVATE_DATA_EXTENSION_NAME)                                 /* Allow object to hold additional 64 bit custom data [EXT_FEAT] [1.3] */ \
	X(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME)        /* Allow access without sync on write to read resources [EXT_PERF] */ \
	X(VK_EXT_RGBA10X6_FORMATS_EXTENSION_NAME)                             /* VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 (PixelFormat::YUV_Y410) format without YCbCr conversion required [EXT_FEAT] */ \
	X(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME)                                 /* Strict out of bound return values and null descriptor handles [EXT_DEBUG] */ \
	X(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME)                             /* Modifying locations of taken samples, useful for antialiasing [EXT_PERF] */ \
	X(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME)                        /* Min-max filters for samplers [EXT_FEAT] [1.2] */ \
	X(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME)                          /* Scalar (C-like) block layout for constant buffers without additional alignment [EXT_SHADER] [1.2] */ \
	X(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME)                       /* Separate stencil buffer usage flags [EXT_FEAT] [1.2] */ \
	X(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME)                          /* Float atomic opertations [EXT_SHADER] */ \
	X(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME)                        /* 16 bit float atomic opertations [EXT_SHADER] */ \
	X(VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME)           /* Shaders can behave as silent workers that won't output anything but still participate in wave ops, better than discard [EXT_SHADER] [EXT_PERF] [1.3] */ \
	X(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME)                    /* 64 bit integer atomic opertations on images [EXT_SHADER] */ \
	X(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME)                     /* Caching compiled at runtime shader modules on disks to be used on subsequent runs [EXT_PERF] [EXT_FEAT] [EXT_SHADER] */ \
	X(VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME)                        /* Allow shader to output stencil ref value [EXT_SHADER] */ \
	X(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME)                  /* Allow vertex/hull shader to select which render target layer or viewport to write to [EXT_SHADER] [EXT_FEAT] [1.2] */ \
	X(VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME)                       /* Better info about texture alignments [EXT_MEMORY] [EXT_QUERY] [1.3] */ \
	X(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME)                 /* ASTC texture compression for HDR [EXT_FEAT] [1.3] */ \
	X(VK_EXT_TOOLING_INFO_EXTENSION_NAME)                                 /* Get info about what tools are currently working [EXT_QUERY] [EXT_FEAT] [EXT_DEBUG] [1.3] */ \
	X(VK_EXT_VALIDATION_CACHE_EXTENSION_NAME)                             /* Caching validation checks for multiple runs [EXT_DEBUG] */ \
	X(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME)                     /* Using same atributes for multiple instances without duplication [EXT_PERF] */ \
	X(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME)                   /* Vertex input binding as dynamic state [EXT_FEAT] [EXT_PERF] */ \
	X(VK_EXT_YCBCR_IMAGE_ARRAYS_EXTENSION_NAME)                           /* Allow arrays for YCbCr images [EXT_FEAT] */ \
	X(VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME)                     /* Additional YCbCr 4:4:4 planar textures [EXT_FEAT] [1.3] */ \
	\
	X(VK_AMD_BUFFER_MARKER_EXTENSION_NAME)                                /* Breadcrumb markers for tracking commands execution in case of TDR [EXT_DEBUG] */ \
	X(VK_AMD_DISPLAY_NATIVE_HDR_EXTENSION_NAME)                           /* Using native display HDR formats (probably FreeSync support too) [EXT_WINDOW] [EXT_FEAT] */ \
	X(VK_AMD_GCN_SHADER_EXTENSION_NAME)                                   /* New shader instructions for cube textures [EXT_SHADER] */ \
	X(VK_AMD_SHADER_BALLOT_EXTENSION_NAME)                                /* New shader instructions for multiple cores [EXT_SHADER] */ \
	X(VK_AMD_SHADER_CORE_PROPERTIES_EXTENSION_NAME)                       /* Get information about GPU cores [EXT_SHADER] [EXT_QUERY] */ \
	X(VK_AMD_SHADER_CORE_PROPERTIES_2_EXTENSION_NAME)                     /* Get extended information about GPU cores [EXT_SHADER] [EXT_QUERY] */ \
	X(VK_AMD_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_EXTENSION_NAME)         /* Special control over early and late pixel tests [EXT_SHADER] */ \
	X(VK_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER_EXTENSION_NAME)             /* New barycentric vertex outputs interpolation [EXT_SHADER] */ \
	X(VK_AMD_SHADER_FRAGMENT_MASK_EXTENSION_NAME)                         /* Access to current pixel mask for multisampled surfaces [EXT_SHADER] */ \
	X(VK_AMD_SHADER_IMAGE_LOAD_STORE_LOD_EXTENSION_NAME)                  /* Expanded LOD on image accesses [EXT_SHADER] */ \
	X(VK_AMD_SHADER_INFO_EXTENSION_NAME)                                  /* Get statistics about compiled shaders [EXT_QUERY] [EXT_SHADER] [EXT_DEBUG] */ \
	X(VK_AMD_SHADER_TRINARY_MINMAX_EXTENSION_NAME)                        /* Faster shader instructions for 3 operands [EXT_SHADER] [EXT_PERF] */ \
	X(VK_AMD_TEXTURE_GATHER_BIAS_LOD_EXTENSION_NAME)                      /* LOD bias when using gather on textures [EXT_SHADER] */ \
	\
	X(VK_ARM_SHADER_CORE_BUILTINS_EXTENSION_NAME)                         /* Get details about ARM GPU's [EXT_SHADER] [EXT_QUERY] */ \
	\
	X(VK_GOOGLE_DECORATE_STRING_EXTENSION_NAME)                           /* Additional decorations for shader modules [EXT_SHADER] */ \
	X(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME)                            /* Get smooth synchronized presentation on mobiles [EXT_WINDOW] [EXT_QUERY] */ \
	X(VK_GOOGLE_HLSL_FUNCTIONALITY1_EXTENSION_NAME)			              /* Some new UAV counter possibilities [EXT_SHADER] [EXT_FEAT] */ \
	X(VK_GOOGLE_USER_TYPE_EXTENSION_NAME)					              /* Decorate variables with type strings (something about reflection probably) [EXT_SHADER] */ \
	\
	X(VK_HUAWEI_INVOCATION_MASK_EXTENSION_NAME)                           /* In case of sparse rays, provide optimization mask [EXT_RT] [EXT_PERF] */ \
	\
	X(VK_INTEL_PERFORMANCE_QUERY_EXTENSION_NAME)                          /* Performance measurments [EXT_QUERY] [EXT_PERF] */ \
	X(VK_INTEL_SHADER_INTEGER_FUNCTIONS_2_EXTENSION_NAME)                 /* New integer instructions similar to x86 [EXT_SHADER] */ \
	\
	X(VK_NV_ACQUIRE_WINRT_DISPLAY_EXTENSION_NAME)                         /* Full control over WinRT display [EXT_WINDOW] */ \
	X(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME)                          /* Creating barrel distortion to rendered image to avoid computing of unused pixels in AR/VR apps [EXT_PERF] */ \
	X(VK_NV_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME)                    /* Computing derivatives [EXT_SHADER] [EXT_PERF] */ \
	X(VK_NV_COOPERATIVE_MATRIX_EXTENSION_NAME)                            /* New smaller matrix type that spreads multiplication on different waves [EXT_SHADER] [EXT_PERF] */ \
	X(VK_NV_COPY_MEMORY_INDIRECT_EXTENSION_NAME)                          /* Indirect copies between into image [EXT_FEAT] */ \
	X(VK_NV_CORNER_SAMPLED_IMAGE_EXTENSION_NAME)                          /* New image format with different sample logic [EXT_FEAT] */ \
	X(VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME)                       /* Control how render target value is computed in VK_NV_framebuffer_mixed_samples [EXT_FEAT] */ \
	X(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME)                 /* Inserting breadcrumb markers for crash post-mortem [EXT_DEBUG] */ \
	X(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME)                     /* Configuring crash dumps with Nsight Aftermath [EXT_DEBUG] */ \
	X(VK_NV_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME)                     /* Indirect drawing [EXT_FEAT] [1.1] */ \
	X(VK_NV_EXTERNAL_MEMORY_RDMA_EXTENSION_NAME)                          /* Accessing external memory between GPUs [EXT_EXMEM] [EXT_FEAT] */ \
	X(VK_NV_FILL_RECTANGLE_EXTENSION_NAME)                                /* Render rectangle with single triangle via bounding box [EXT_FEAT] */ \
	X(VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME)                    /* Output coverage after stencils and alpha-to-coverage steps, useful in determining some data of original primitive [EXT_FEAT] */ \
	X(VK_NV_FRAGMENT_SHADING_RATE_ENUMS_EXTENSION_NAME)                   /* Control rate of pixel shading done on VRS via shading rate [EXT_PERF] */ \
	X(VK_NV_GEOMETRY_SHADER_PASSTHROUGH_EXTENSION_NAME)                   /* Faster passing of unchanged vertex data in GS [EXT_SHADER] [EXT_PERF] */ \
	X(VK_NV_INHERITED_VIEWPORT_SCISSOR_EXTENSION_NAME)                    /* Inheriting viewport and scissors for secondary command list [EXT_PERF] */ \
	X(VK_NV_LINEAR_COLOR_ATTACHMENT_EXTENSION_NAME)                       /* Using lineary tiled textures as render targets [EXT_FEAT] */ \
	X(VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME)                          /* Performing direct GPU decompression [EXT_MEMORY] [EXT_FEAT] */ \
	X(VK_NV_OPTICAL_FLOW_EXTENSION_NAME)                                  /* Easier displacement between pixels (maybe for motion vectors) [EXT_FEAT] */ \
	X(VK_NV_PRESENT_BARRIER_EXTENSION_NAME)                               /* Synchronizing presentation between multiple swapchains [EXT_WINDOW] */ \
	X(VK_NV_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME)                /* Shader execution reordering for rays [EXT_RT] [EXT_PERF] */ \
	X(VK_NV_RAY_TRACING_MOTION_BLUR_EXTENSION_NAME)                       /* Faster tracing of geometry in motion [EXT_RT] [EXT_PERF] */ \
	X(VK_NV_SAMPLE_MASK_OVERRIDE_COVERAGE_EXTENSION_NAME)                 /* Control which samples are used to process pixel [EXT_SHADER] */ \
	X(VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME)                             /* Opposite scissor test [EXT_FEAT] */ \
	X(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME)                            /* Specifying image controlling pixel shading amount (VRS) [EXT_FEAT] */ \
	X(VK_NV_SHADER_IMAGE_FOOTPRINT_EXTENSION_NAME)                        /* Gather info about processed pixels to not waste it on other when sampling textures [EXT_SHADER] [EXT_FEAT] */ \
	X(VK_NV_SHADER_SM_BUILTINS_EXTENSION_NAME)                            /* Info about shader processors [EXT_SHADER] [EXT_QUERY] */ \
	X(VK_NV_SHADER_SUBGROUP_PARTITIONED_EXTENSION_NAME)                   /* Information about some other waves [EXT_SHADER] */ \
	X(VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME)                               /* Allow single primitive to be broadcasted to multiple viewports [EXT_PERF] [EXT_SHADER] */ \
	X(VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME)                              /* Reorienting vertex data for different bound viewports (usable in single pass cubemap rendering) [EXT_PERF] [EXT_SHADER] */ \
	\
	X(VK_NVX_BINARY_IMPORT_EXTENSION_NAME)                                /* Importing CUDA binaries [EXT_FEAT] */ \
	X(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME)                            /* Get image handles from views */ \
	X(VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME)                /* Multiview output for subpasses [EXT_FEAT] */ \
	\
	X(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME)                 /* Allow offsets for VRS density maps [EXT_FEAT] [EXT_PERF] */ \
	X(VK_QCOM_RENDER_PASS_SHADER_RESOLVE_EXTENSION_NAME)                  /* Customizable resolve [EXT_FEAT] [EXT_SHADER] */ \
	X(VK_QCOM_RENDER_PASS_TRANSFORM_EXTENSION_NAME)                       /* In AR/VR apps when device is rotated, apply said transformation directly in the driver [EXT_PERF] */ \
	X(VK_QCOM_ROTATED_COPY_COMMANDS_EXTENSION_NAME)                       /* In AR/VR when using rotated buffers, take that into account during copying [EXT_FEAT] */ \
	X(VK_QCOM_TILE_PROPERTIES_EXTENSION_NAME)                             /* Get properties of single tile used by rendering [EXT_FEAT] [EXT_QUERY] */ \
	X(VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME)                            /* New instructions for better processing of images [EXT_SHADER] [EXT_FEAT] */ \
	\
	X(VK_VALVE_DESCRIPTOR_SET_HOST_MAPPING_EXTENSION_NAME)                /* Easier copying descriptors via memcpy [EXT_PERF] [EXT_FEAT] */

// List of device extension names, intended for use in X() macro
#define ZE_VK_EXT_LIST_DEVICE ZE_VK_EXT_LIST_DEVICE_REQUIRED ZE_VK_EXT_LIST_DEVICE_DEBUG ZE_VK_EXT_LIST_DEVICE_OPTIONAL
#pragma endregion

// List of extension names, intended for use in X() macro
#define ZE_VK_EXT_LIST ZE_VK_EXT_LIST_INSTANCE ZE_VK_EXT_LIST_DEVICE

#pragma region Deprecated and not supported extensions
// Defines for sake of simplicity in deprecated list
#if !VK_KHR_win32_keyed_mutex
#	define VK_KHR_WIN32_KEYED_MUTEX_EXTENSION_NAME "VK_KHR_win32_keyed_mutex"
#endif
#if !VK_EXT_metal_objects
#	define VK_EXT_METAL_OBJECTS_EXTENSION_NAME "VK_EXT_metal_objects"
#endif
#if !VK_EXT_metal_surface
#	define VK_EXT_METAL_SURFACE_EXTENSION_NAME "VK_EXT_metal_surface"
#endif
#if !VK_GGP_stream_descriptor_surface
#	define VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME "VK_GGP_stream_descriptor_surface"
#endif
#if !VK_GGP_frame_token
#	define VK_GGP_FRAME_TOKEN_EXTENSION_NAME "VK_GGP_frame_token"
#endif
#if !VK_MVK_ios_surface
#	define VK_MVK_IOS_SURFACE_EXTENSION_NAME "VK_MVK_ios_surface"
#endif
#if !VK_MVK_macos_surface
#	define VK_MVK_MACOS_SURFACE_EXTENSION_NAME "VK_MVK_macos_surface"
#endif
#if !VK_NV_external_memory_win32
#	define VK_NV_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME "VK_NV_external_memory_win32"
#endif
#if !VK_NV_win32_keyed_mutex
#	define VK_NV_WIN32_KEYED_MUTEX_EXTENSION_NAME "VK_NV_win32_keyed_mutex"
#endif
#if !VK_QNX_screen_surface
#	define VK_QNX_SCREEN_SURFACE_EXTENSION_NAME "VK_QNX_screen_surface"
#endif

// List of deprecated extension names, purely informational to keep track of known extensions, intended for use in X() macro
#define ZE_VK_EXT_LIST_DEPRECATED \
	X(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)       /* Deprecated by VK_KHR_buffer_device_address */ \
	X(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)                /* Promoted to VK_EXT_debug_utils */ \
	X(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)                /* Deprecated by VK_EXT_debug_utils */ \
	X(VK_EXT_GLOBAL_PRIORITY_EXTENSION_NAME)             /* Deprecated by VK_KHR_global_priority */ \
	X(VK_EXT_GLOBAL_PRIORITY_QUERY_EXTENSION_NAME)       /* Promoted to VK_KHR_global_priority */ \
	X(VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME)      /* Superseded by core 1.2 */ \
	X(VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME)        /* Superseded by core 1.1 */ \
	X(VK_EXT_VALIDATION_FLAGS_EXTENSION_NAME)            /* Deprecated by VK_EXT_validation_features */ \
	X(VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME)         /* Promoted to VK_KHR_draw_indirect_count */ \
	X(VK_AMD_GPU_SHADER_HALF_FLOAT_EXTENSION_NAME)       /* Deprecated by VK_KHR_shader_float16_int8 */ \
	X(VK_AMD_GPU_SHADER_INT16_EXTENSION_NAME)            /* Deprecated by VK_KHR_shader_float16_int8 */ \
	X(VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME)    /* Obsoleted by VK_KHR_maintenance1 */ \
	X(VK_IMG_FILTER_CUBIC_EXTENSION_NAME)                /* Extended by VK_EXT_filter_cubic */ \
	X(VK_IMG_FORMAT_PVRTC_EXTENSION_NAME)                /* Deprecated without replacement */ \
	X(VK_MVK_IOS_SURFACE_EXTENSION_NAME)                 /* Deprecated by VK_EXT_metal_surface */ \
	X(VK_MVK_MACOS_SURFACE_EXTENSION_NAME)               /* Deprecated by VK_EXT_metal_surface */ \
	X(VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME)         /* Deprecated by VK_KHR_dedicated_allocation */ \
	X(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME)              /* Deprecated by VK_KHR_external_memory */ \
	X(VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME) /* Deprecated by VK_KHR_external_memory_capabilities */ \
	X(VK_NV_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME)        /* Deprecated by VK_KHR_external_memory_win32 */ \
	X(VK_NV_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME)  /* Promoted to VK_KHR_fragment_shader_barycentric */ \
	X(VK_NV_GLSL_SHADER_EXTENSION_NAME)                  /* Deprecated without replacement */ \
	X(VK_NV_MESH_SHADER_EXTENSION_NAME)                  /* Replaced by VK_EXT_mesh_shader */ \
	X(VK_NV_RAY_TRACING_EXTENSION_NAME)                  /* Replaced by VK_KHR_acceleration_structure, VK_KHR_ray_tracing_pipeline and VK_KHR_ray_query */ \
	X(VK_NV_WIN32_KEYED_MUTEX_EXTENSION_NAME)            /* Promoted to VK_KHR_win32_keyed_mutex */ \
	X(VK_QCOM_RENDER_PASS_STORE_OPS_EXTENSION_NAME)      /* Replaced by VK_KHR_dynamic_rendering */ \
	X(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME)   /* Promoted to VK_EXT_mutable_descriptor_type */

// List of not supported extension names, that won't be added without clear reason, intended for use in X() macro
#define ZE_VK_EXT_LIST_NOT_SUPPORTED \
	X(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)          /* Vulkan 1.3 required, not allowing any subset implementations */ \
	X(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)     /* Vulkan 1.3 required, not allowing any non-conformant devices */ \
	X(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME)          /* Video extensions currently in provisional state, waiting for release [EXT_VIDEO] */ \
	X(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)          /* Video extensions currently in provisional state, waiting for release [EXT_VIDEO] */ \
	X(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME)                 /* Video extensions currently in provisional state, waiting for release [EXT_VIDEO] */ \
	X(VK_KHR_WIN32_KEYED_MUTEX_EXTENSION_NAME)           /* No interop between D3D11 is in plans */ \
	X(VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME)          /* Standard used depth range is [0;1] not [-1;1] */ \
	X(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME)         /* 3D images should be accessed only be 3D views */ \
	X(VK_EXT_LEGACY_DITHERING_EXTENSION_NAME)            /* Not targeting legacy behavior */ \
	X(VK_EXT_METAL_OBJECTS_EXTENSION_NAME)               /* Apple targets not supported */ \
	X(VK_EXT_METAL_SURFACE_EXTENSION_NAME)               /* Apple targets not supported */ \
	X(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME)            /* No reason to change default provoking vertex in APIs */ \
	X(VK_EXT_SUBPASS_MERGE_FEEDBACK_EXTENSION_NAME)      /* Render passes are not the emphasized way of using the API, instead dynamic rendering is required */ \
	X(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME)          /* Discouraged to use, instead choose other vertex data capture methods */ \
	X(VK_EXT_VIDEO_DECODE_H264_EXTENSION_NAME)           /* Video extensions currently in provisional state, waiting for release [EXT_VIDEO] */ \
	X(VK_EXT_VIDEO_DECODE_H265_EXTENSION_NAME)           /* Video extensions currently in provisional state, waiting for release [EXT_VIDEO] */ \
	X(VK_EXT_VIDEO_ENCODE_H264_EXTENSION_NAME)           /* Video extensions currently in provisional state, waiting for release [EXT_VIDEO] */ \
	X(VK_EXT_VIDEO_ENCODE_H265_EXTENSION_NAME)           /* Video extensions currently in provisional state, waiting for release [EXT_VIDEO] */ \
	X(VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME)   /* Google Stadia is discontinued */ \
	X(VK_GGP_FRAME_TOKEN_EXTENSION_NAME)                 /* Google Stadia is discontinued */ \
	X(VK_HUAWEI_SUBPASS_SHADING_EXTENSION_NAME)          /* Subpasses included in old render passes are not supported */ \
	X(VK_QNX_SCREEN_SURFACE_EXTENSION_NAME)              /* QNX target is not planned for support */ \
	X(VK_SEC_AMIGO_PROFILING_EXTENSION_NAME)             /* Discouraged to use, functionality mostly for tools and layers */ \
	ZE_VK_EXT_LIST_DEPRECATED
#pragma endregion