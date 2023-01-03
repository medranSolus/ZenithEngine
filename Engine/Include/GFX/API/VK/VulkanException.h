#pragma once
#include "Exception/BasicException.h"
#include "VK.h"

namespace ZE::GFX::API::VK
{
	// Standard exception for Vulkan layer
	class VulkanException : public Exception::BasicException
	{
		VkResult result;

	public:
		VulkanException(U32 line, const char* file, VkResult result) noexcept
			: BasicException(line, file), result(result) {}
		ZE_CLASS_DEFAULT(VulkanException);
		virtual ~VulkanException() = default;

		static constexpr const char* TranslateResult(VkResult result) noexcept;

		constexpr const char* GetType() const noexcept override { return "Vulkan Graphics Exception"; }

		const char* what() const noexcept override;
	};

#pragma region Functions
	constexpr const char* VulkanException::TranslateResult(VkResult result) noexcept
	{
#define DECODE_RESULT(res) case res: return #res
		switch (result)
		{
			DECODE_RESULT(VK_SUCCESS);
			DECODE_RESULT(VK_NOT_READY);
			DECODE_RESULT(VK_TIMEOUT);
			DECODE_RESULT(VK_EVENT_SET);
			DECODE_RESULT(VK_EVENT_RESET);
			DECODE_RESULT(VK_INCOMPLETE);
			DECODE_RESULT(VK_ERROR_OUT_OF_HOST_MEMORY);
			DECODE_RESULT(VK_ERROR_OUT_OF_DEVICE_MEMORY);
			DECODE_RESULT(VK_ERROR_INITIALIZATION_FAILED);
			DECODE_RESULT(VK_ERROR_DEVICE_LOST);
			DECODE_RESULT(VK_ERROR_MEMORY_MAP_FAILED);
			DECODE_RESULT(VK_ERROR_LAYER_NOT_PRESENT);
			DECODE_RESULT(VK_ERROR_EXTENSION_NOT_PRESENT);
			DECODE_RESULT(VK_ERROR_FEATURE_NOT_PRESENT);
			DECODE_RESULT(VK_ERROR_INCOMPATIBLE_DRIVER);
			DECODE_RESULT(VK_ERROR_TOO_MANY_OBJECTS);
			DECODE_RESULT(VK_ERROR_FORMAT_NOT_SUPPORTED);
			DECODE_RESULT(VK_ERROR_FRAGMENTED_POOL);
			DECODE_RESULT(VK_ERROR_UNKNOWN);
			DECODE_RESULT(VK_ERROR_OUT_OF_POOL_MEMORY);
			DECODE_RESULT(VK_ERROR_INVALID_EXTERNAL_HANDLE);
			DECODE_RESULT(VK_ERROR_FRAGMENTATION);
			DECODE_RESULT(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
			DECODE_RESULT(VK_PIPELINE_COMPILE_REQUIRED);
			DECODE_RESULT(VK_ERROR_SURFACE_LOST_KHR);
			DECODE_RESULT(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
			DECODE_RESULT(VK_SUBOPTIMAL_KHR);
			DECODE_RESULT(VK_ERROR_OUT_OF_DATE_KHR);
			DECODE_RESULT(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
			DECODE_RESULT(VK_ERROR_VALIDATION_FAILED_EXT);
			DECODE_RESULT(VK_ERROR_INVALID_SHADER_NV);
#ifdef VK_ENABLE_BETA_EXTENSIONS
			DECODE_RESULT(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR);
			DECODE_RESULT(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR);
			DECODE_RESULT(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR);
			DECODE_RESULT(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR);
			DECODE_RESULT(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR);
			DECODE_RESULT(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR);
#endif
			DECODE_RESULT(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
			DECODE_RESULT(VK_ERROR_NOT_PERMITTED_KHR);
			DECODE_RESULT(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
			DECODE_RESULT(VK_THREAD_IDLE_KHR);
			DECODE_RESULT(VK_THREAD_DONE_KHR);
			DECODE_RESULT(VK_OPERATION_DEFERRED_KHR);
			DECODE_RESULT(VK_OPERATION_NOT_DEFERRED_KHR);
			DECODE_RESULT(VK_ERROR_COMPRESSION_EXHAUSTED_EXT);
		default:
			return "UNKNOWN_RESULT";
		}
#undef DECODE_ERROR
	}
#pragma endregion
}

#pragma region Exception macros
// Variable holding result of last Vulkan call
#define ZE_VK_EXCEPT_RESULT __vkResult
#define	ZE_VK_EXCEPT(result) ZE::GFX::API::VK::VulkanException(__LINE__, __FILENAME__, result)

// Enables useage of ZE_VK_* macros in current scope
#define ZE_VK_ENABLE() VkResult ZE_VK_EXCEPT_RESULT

// Before using needs call to ZE_VK_ENABLE()
// Checks VkResult returned via function and throws on error
#define	ZE_VK_THROW_FAILED(call) if((ZE_VK_EXCEPT_RESULT = (call)) < VK_SUCCESS) throw ZE_VK_EXCEPT(ZE_VK_EXCEPT_RESULT)

// Before using needs call to ZE_VK_ENABLE()
// Checks VkResult returned via function and throws when no returned VK_SUCCESS
#define	ZE_VK_THROW_NOSUCC(call) if((ZE_VK_EXCEPT_RESULT = (call)) != VK_SUCCESS) throw ZE_VK_EXCEPT(ZE_VK_EXCEPT_RESULT)
#pragma endregion

#pragma region Debug name macros
// Variable name holding debug name
#define ZE_VK_DEBUG_ID __debugObjectInfo
#define ZE_VK_DEBUG_ID_STRING __debugObjectInfoName

#if _ZE_DEBUG_GFX_NAMES
// Enables useage of ZE_VK_SET_ID macros in current scope
#define ZE_VK_ENABLE_ID() ZE_VK_ENABLE(); VkDebugUtilsObjectNameInfoEXT ZE_VK_DEBUG_ID = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, nullptr }; std::string ZE_VK_DEBUG_ID_STRING

// Before using needs call to ZE_VK_ENABLE_ID()
// Sets debug name for GPU object with given id
#define ZE_VK_SET_ID(device, object, vkObjectType, id) ZE_VK_DEBUG_ID.objectType = vkObjectType; ZE_VK_DEBUG_ID.objectHandle = (U64)(object); ZE_VK_DEBUG_ID_STRING = id; ZE_VK_DEBUG_ID.pObjectName = ZE_VK_DEBUG_ID_STRING.c_str(); ZE_VK_THROW_NOSUCC(vkSetDebugUtilsObjectNameEXT(device, &ZE_VK_DEBUG_ID))

#else
// Enables useage of ZE_VK_SET_ID macros in current scope
#define ZE_VK_ENABLE_ID() ZE_VK_ENABLE()

// Before using needs call to ZE_VK_ENABLE_ID()
// Sets debug name for GPU object with given id
#define ZE_VK_SET_ID(device, object, vkObjectType, id)
#endif
#pragma endregion