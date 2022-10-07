#include "GFX/API/VK/Device.h"
#include "GFX/API/VK/VulkanException.h"
#include "GFX/CommandList.h"

namespace ZE::GFX::API::VK
{
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
		uint32_t instanceVersion = 0;
		ZE_VK_THROW_NOSUCC(vkEnumerateInstanceVersion(&instanceVersion));
		if (instanceVersion < appInfo.apiVersion)
		{
			throw ZE_CMP_EXCEPT("Vulkan instance version reported is not supported on this engine!\nRequired version: "
				+ std::to_string(appInfo.apiVersion) + "\nPresent version: " + std::to_string(instanceVersion));
		}

		/* 1.3 ext:
		* VK_KHR_copy_commands2
		* VK_KHR_dynamic_rendering
		* VK_KHR_format_feature_flags2
		* VK_KHR_maintenance4
		* VK_KHR_shader_integer_dot_product
		* VK_KHR_shader_non_semantic_info
		* VK_KHR_shader_terminate_invocation
		* VK_KHR_synchronization2
		* VK_KHR_zero_initialize_workgroup_memory
		* VK_EXT_4444_formats
		* VK_EXT_extended_dynamic_state
		* VK_EXT_extended_dynamic_state2
		* VK_EXT_image_robustness
		* VK_EXT_inline_uniform_block
		* VK_EXT_pipeline_creation_cache_control
		* VK_EXT_pipeline_creation_feedback
		* VK_EXT_private_data
		* VK_EXT_shader_demote_to_helper_invocation
		* VK_EXT_subgroup_size_control
		* VK_EXT_texel_buffer_alignment
		* VK_EXT_texture_compression_astc_hdr
		* VK_EXT_tooling_info
		* VK_EXT_ycbcr_2plane_444_formats
		*
		* 1.2 ext:
		* VK_KHR_8bit_storage
		* VK_KHR_buffer_device_address
		* VK_KHR_create_renderpass2
		* VK_KHR_depth_stencil_resolve
		* VK_KHR_draw_indirect_count
		* VK_KHR_driver_properties
		* VK_KHR_image_format_list
		* VK_KHR_imageless_framebuffer
		* VK_KHR_sampler_mirror_clamp_to_edge
		* VK_KHR_separate_depth_stencil_layouts
		* VK_KHR_shader_atomic_int64
		* VK_KHR_shader_float16_int8
		* VK_KHR_shader_float_controls
		* VK_KHR_shader_subgroup_extended_types
		* VK_KHR_spirv_1_4
		* VK_KHR_timeline_semaphore
		* VK_KHR_uniform_buffer_standard_layout
		* VK_KHR_vulkan_memory_model
		* VK_EXT_descriptor_indexing
		* VK_EXT_host_query_reset
		* VK_EXT_sampler_filter_minmax
		* VK_EXT_scalar_block_layout
		* VK_EXT_separate_stencil_usage
		* VK_EXT_shader_viewport_index_layer
		*
		* 1.1 ext:
		* VK_KHR_16bit_storage
		* VK_KHR_bind_memory2
		* VK_KHR_dedicated_allocation
		* VK_KHR_descriptor_update_template
		* VK_KHR_device_group
		* VK_KHR_device_group_creation
		* VK_KHR_external_fence
		* VK_KHR_external_fence_capabilities
		* VK_KHR_external_memory
		* VK_KHR_external_memory_capabilities
		* VK_KHR_external_semaphore
		* VK_KHR_external_semaphore_capabilities
		* VK_KHR_get_memory_requirements2
		* VK_KHR_get_physical_device_properties2
		* VK_KHR_maintenance1
		* VK_KHR_maintenance2
		* VK_KHR_maintenance3
		* VK_KHR_multiview
		* VK_KHR_relaxed_block_layout
		* VK_KHR_sampler_ycbcr_conversion
		* VK_KHR_shader_draw_parameters
		* VK_KHR_storage_buffer_storage_class
		* VK_KHR_variable_pointers
		*/

		/* All ext:
		* VK_KHR_acceleration_structure
		* VK_KHR_android_surface
		* VK_KHR_deferred_host_operations
		* VK_KHR_display
		* VK_KHR_display_swapchain
		* VK_KHR_external_fence_fd
		* VK_KHR_external_fence_win32
		* VK_KHR_external_memory_fd
		* VK_KHR_external_memory_win32
		* VK_KHR_external_semaphore_fd
		* VK_KHR_external_semaphore_win32
		* VK_KHR_fragment_shader_barycentric
		* VK_KHR_fragment_shading_rate
		* VK_KHR_get_display_properties2
		* VK_KHR_get_surface_capabilities2
		* VK_KHR_global_priority
		* VK_KHR_incremental_present
		* VK_KHR_performance_query
		* VK_KHR_pipeline_executable_properties
		* VK_KHR_pipeline_library
		* VK_KHR_portability_enumeration
		* VK_KHR_present_id
		* VK_KHR_present_wait
		* VK_KHR_push_descriptor
		* VK_KHR_ray_query
		* VK_KHR_ray_tracing_maintenance1
		* VK_KHR_ray_tracing_pipeline
		* VK_KHR_shader_clock
		* VK_KHR_shader_subgroup_uniform_control_flow
		* VK_KHR_shared_presentable_image
		* VK_KHR_surface
		* VK_KHR_surface_protected_capabilities
		* VK_KHR_swapchain
		* VK_KHR_swapchain_mutable_format
		* VK_KHR_wayland_surface
		* VK_KHR_win32_keyed_mutex
		* VK_KHR_win32_surface
		* VK_KHR_workgroup_memory_explicit_layout
		* VK_KHR_xcb_surface
		* VK_KHR_xlib_surface
		* VK_EXT_acquire_drm_display
		* VK_EXT_acquire_xlib_display
		* VK_EXT_astc_decode_mode
		* VK_EXT_attachment_feedback_loop_layout
		* VK_EXT_blend_operation_advanced
		* VK_EXT_border_color_swizzle
		* VK_EXT_calibrated_timestamps
		* VK_EXT_color_write_enable
		* VK_EXT_conditional_rendering
		* VK_EXT_conservative_rasterization
		* VK_EXT_custom_border_color
		* VK_EXT_debug_utils
		* VK_EXT_depth_clamp_zero_one
		* VK_EXT_depth_clip_control
		* VK_EXT_depth_clip_enable
		* VK_EXT_depth_range_unrestricted
		* VK_EXT_device_address_binding_report
		* VK_EXT_device_fault
		* VK_EXT_device_memory_report
		* VK_EXT_direct_mode_display
		* VK_EXT_directfb_surface
		* VK_EXT_discard_rectangles
		* VK_EXT_display_control
		* VK_EXT_display_surface_counter
		* VK_EXT_extended_dynamic_state3
		* VK_EXT_external_memory_dma_buf
		* VK_EXT_external_memory_host
		* VK_EXT_filter_cubic
		* VK_EXT_fragment_density_map
		* VK_EXT_fragment_density_map2
		* VK_EXT_fragment_shader_interlock
		* VK_EXT_full_screen_exclusive
		* VK_EXT_graphics_pipeline_library
		* VK_EXT_hdr_metadata
		* VK_EXT_headless_surface
		* VK_EXT_image_2d_view_of_3d
		* VK_EXT_image_compression_control
		* VK_EXT_image_compression_control_swapchain
		* VK_EXT_image_drm_format_modifier
		* VK_EXT_image_view_min_lod
		* VK_EXT_index_type_uint8
		* VK_EXT_legacy_dithering
		* VK_EXT_line_rasterization
		* VK_EXT_load_store_op_none
		* VK_EXT_memory_budget
		* VK_EXT_memory_priority
		* VK_EXT_mesh_shader
		* VK_EXT_metal_objects
		* VK_EXT_metal_surface
		* VK_EXT_multi_draw
		* VK_EXT_multisampled_render_to_single_sampled
		* VK_EXT_mutable_descriptor_type
		* VK_EXT_non_seamless_cube_map
		* VK_EXT_opacity_micromap
		* VK_EXT_pageable_device_local_memory
		* VK_EXT_pci_bus_info
		* VK_EXT_physical_device_drm
		* VK_EXT_pipeline_properties
		* VK_EXT_pipeline_protected_access
		* VK_EXT_pipeline_robustness
		* VK_EXT_post_depth_coverage
		* VK_EXT_primitive_topology_list_restart
		* VK_EXT_primitives_generated_query
		* VK_EXT_provoking_vertex
		* VK_EXT_queue_family_foreign
		* VK_EXT_rasterization_order_attachment_access
		* VK_EXT_rgba10x6_formats
		* VK_EXT_robustness2
		* VK_EXT_sample_locations
		* VK_EXT_shader_atomic_float
		* VK_EXT_shader_atomic_float2
		* VK_EXT_shader_image_atomic_int64
		* VK_EXT_shader_module_identifier
		* VK_EXT_shader_stencil_export
		* VK_EXT_subpass_merge_feedback
		* VK_EXT_swapchain_colorspace
		* VK_EXT_transform_feedback
		* VK_EXT_validation_cache
		* VK_EXT_validation_features
		* VK_EXT_vertex_attribute_divisor
		* VK_EXT_vertex_input_dynamic_state
		* VK_EXT_ycbcr_image_arrays
		* VK_AMD_buffer_marker
		* VK_AMD_device_coherent_memory
		* VK_AMD_display_native_hdr
		* VK_AMD_gcn_shader
		* VK_AMD_memory_overallocation_behavior
		* VK_AMD_mixed_attachment_samples
		* VK_AMD_pipeline_compiler_control
		* VK_AMD_rasterization_order
		* VK_AMD_shader_ballot
		* VK_AMD_shader_core_properties
		* VK_AMD_shader_core_properties2
		* VK_AMD_shader_early_and_late_fragment_tests
		* VK_AMD_shader_explicit_vertex_parameter
		* VK_AMD_shader_fragment_mask
		* VK_AMD_shader_image_load_store_lod
		* VK_AMD_shader_info
		* VK_AMD_shader_trinary_minmax
		* VK_AMD_texture_gather_bias_lod
		* VK_ANDROID_external_memory_android_hardware_buffer
		* VK_FUCHSIA_buffer_collection
		* VK_FUCHSIA_external_memory
		* VK_FUCHSIA_external_semaphore
		* VK_FUCHSIA_imagepipe_surface
		* VK_GGP_frame_token
		* VK_GGP_stream_descriptor_surface
		* VK_GOOGLE_decorate_string
		* VK_GOOGLE_display_timing
		* VK_GOOGLE_hlsl_functionality1
		* VK_GOOGLE_surfaceless_query
		* VK_GOOGLE_user_type
		* VK_HUAWEI_invocation_mask
		* VK_HUAWEI_subpass_shading
		* VK_IMG_filter_cubic
		* VK_IMG_format_pvrtc
		* VK_INTEL_performance_query
		* VK_INTEL_shader_integer_functions2
		* VK_NN_vi_surface
		* VK_NV_acquire_winrt_display
		* VK_NV_clip_space_w_scaling
		* VK_NV_compute_shader_derivatives
		* VK_NV_cooperative_matrix
		* VK_NV_corner_sampled_image
		* VK_NV_coverage_reduction_mode
		* VK_NV_dedicated_allocation_image_aliasing
		* VK_NV_device_diagnostic_checkpoints
		* VK_NV_device_diagnostics_config
		* VK_NV_device_generated_commands
		* VK_NV_external_memory_rdma
		* VK_NV_fill_rectangle
		* VK_NV_fragment_coverage_to_color
		* VK_NV_fragment_shading_rate_enums
		* VK_NV_framebuffer_mixed_samples
		* VK_NV_geometry_shader_passthrough
		* VK_NV_inherited_viewport_scissor
		* VK_NV_linear_color_attachment
		* VK_NV_mesh_shader
		* VK_NV_optical_flow
		* VK_NV_present_barrier
		* VK_NV_ray_tracing
		* VK_NV_ray_tracing_motion_blur
		* VK_NV_representative_fragment_test
		* VK_NV_sample_mask_override_coverage
		* VK_NV_scissor_exclusive
		* VK_NV_shader_image_footprint
		* VK_NV_shader_sm_builtins
		* VK_NV_shader_subgroup_partitioned
		* VK_NV_shading_rate_image
		* VK_NV_viewport_array2
		* VK_NV_viewport_swizzle
		* VK_NVX_binary_import
		* VK_NVX_image_view_handle
		* VK_NVX_multiview_per_view_attributes
		* VK_QCOM_fragment_density_map_offset
		* VK_QCOM_image_processing
		* VK_QCOM_render_pass_shader_resolve
		* VK_QCOM_render_pass_store_ops
		* VK_QCOM_render_pass_transform
		* VK_QCOM_rotated_copy_commands
		* VK_QCOM_tile_properties
		* VK_QNX_screen_surface
		* VK_SEC_amigo_profiling
		* VK_VALVE_descriptor_set_host_mapping
		*/

		/* Provisional ext:
		* VK_KHR_portability_subset
		* VK_KHR_video_decode_queue
		* VK_KHR_video_encode_queue
		* VK_KHR_video_queue
		* VK_EXT_video_decode_h264
		* VK_EXT_video_decode_h265
		* VK_EXT_video_encode_h264
		* VK_EXT_video_encode_h265
		*/

		/* Deprecated ext:
		* VK_KHR_16bit_storage
		* VK_KHR_8bit_storage
		* VK_KHR_bind_memory2
		* VK_KHR_buffer_device_address
		* VK_KHR_copy_commands2
		* VK_KHR_create_renderpass2
		* VK_KHR_dedicated_allocation
		* VK_KHR_depth_stencil_resolve
		* VK_KHR_descriptor_update_template
		* VK_KHR_device_group
		* VK_KHR_device_group_creation
		* VK_KHR_draw_indirect_count
		* VK_KHR_driver_properties
		* VK_KHR_dynamic_rendering
		* VK_KHR_external_fence
		* VK_KHR_external_fence_capabilities
		* VK_KHR_external_memory
		* VK_KHR_external_memory_capabilities
		* VK_KHR_external_semaphore
		* VK_KHR_external_semaphore_capabilities
		* VK_KHR_format_feature_flags2
		* VK_KHR_get_memory_requirements2
		* VK_KHR_get_physical_device_properties2
		* VK_KHR_image_format_list
		* VK_KHR_imageless_framebuffer
		* VK_KHR_maintenance1
		* VK_KHR_maintenance2
		* VK_KHR_maintenance3
		* VK_KHR_maintenance4
		* VK_KHR_multiview
		* VK_KHR_relaxed_block_layout
		* VK_KHR_sampler_mirror_clamp_to_edge
		* VK_KHR_sampler_ycbcr_conversion
		* VK_KHR_separate_depth_stencil_layouts
		* VK_KHR_shader_atomic_int64
		* VK_KHR_shader_draw_parameters
		* VK_KHR_shader_float16_int8
		* VK_KHR_shader_float_controls
		* VK_KHR_shader_integer_dot_product
		* VK_KHR_shader_non_semantic_info
		* VK_KHR_shader_subgroup_extended_types
		* VK_KHR_shader_terminate_invocation
		* VK_KHR_spirv_1_4
		* VK_KHR_storage_buffer_storage_class
		* VK_KHR_synchronization2
		* VK_KHR_timeline_semaphore
		* VK_KHR_uniform_buffer_standard_layout
		* VK_KHR_variable_pointers
		* VK_KHR_vulkan_memory_model
		* VK_KHR_zero_initialize_workgroup_memory
		* VK_EXT_4444_formats
		* VK_EXT_buffer_device_address
		* VK_EXT_debug_marker
		* VK_EXT_debug_report
		* VK_EXT_descriptor_indexing
		* VK_EXT_extended_dynamic_state
		* VK_EXT_extended_dynamic_state2
		* VK_EXT_global_priority
		* VK_EXT_global_priority_query
		* VK_EXT_host_query_reset
		* VK_EXT_image_robustness
		* VK_EXT_inline_uniform_block
		* VK_EXT_pipeline_creation_cache_control
		* VK_EXT_pipeline_creation_feedback
		* VK_EXT_private_data
		* VK_EXT_sampler_filter_minmax
		* VK_EXT_scalar_block_layout
		* VK_EXT_separate_stencil_usage
		* VK_EXT_shader_demote_to_helper_invocation
		* VK_EXT_shader_subgroup_ballot
		* VK_EXT_shader_subgroup_vote
		* VK_EXT_shader_viewport_index_layer
		* VK_EXT_subgroup_size_control
		* VK_EXT_texel_buffer_alignment
		* VK_EXT_texture_compression_astc_hdr
		* VK_EXT_tooling_info
		* VK_EXT_validation_flags
		* VK_EXT_ycbcr_2plane_444_formats
		* VK_AMD_draw_indirect_count
		* VK_AMD_gpu_shader_half_float
		* VK_AMD_gpu_shader_int16
		* VK_AMD_negative_viewport_height
		* VK_ARM_rasterization_order_attachment_access
		* VK_MVK_ios_surface
		* VK_MVK_macos_surface
		* VK_NV_dedicated_allocation
		* VK_NV_external_memory
		* VK_NV_external_memory_capabilities
		* VK_NV_external_memory_win32
		* VK_NV_fragment_shader_barycentric
		* VK_NV_glsl_shader
		* VK_NV_win32_keyed_mutex
		* VK_VALVE_mutable_descriptor_type
		*/

		// Specify features for type of build
		VkValidationFeaturesEXT validationFeatures = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, nullptr };
#ifdef _ZE_MODE_DEBUG
		//VkDebugUtilsMessengerCreateInfoEXT debugMessenger = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, nullptr };
		//debugMessenger.flags = 0;
		//debugMessenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		//debugMessenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		//// if VK_EXT_device_address_binding_report then VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT
		//debugMessenger.pfnUserCallback = nullptr;
		//debugMessenger.pUserData = nullptr;

		//validationFeatures.pNext = &debugMessenger;

		const VkValidationFeatureEnableEXT enabledValidations[] =
		{
#ifdef _ZE_DEBUG_GPU_VALIDATION
			VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
			VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
#endif
			VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
			VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
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

		VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, &validationFeatures };
		instanceInfo.flags = 0;
		instanceInfo.pApplicationInfo = &appInfo;

		instanceInfo.enabledLayerCount;
		instanceInfo.ppEnabledLayerNames;
		instanceInfo.enabledExtensionCount;
		instanceInfo.ppEnabledExtensionNames;

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