#pragma once
#include "VK.h"
#include <bitset>

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::GFX::API::VK
{
	class Device final
	{
#define X(ext) + 1
		static constexpr U16 KNOWN_EXTENSION_COUNT = ZE_VK_EXT_LIST;
#undef X

		VkInstance instance;
		VkDevice device;
		std::bitset<KNOWN_EXTENSION_COUNT + 1> extensionSupport;

#if _ZE_DEBUG_GFX_API
		static VkBool32 VKAPI_PTR DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
#endif
		template<U64 Size>
		static constexpr U16 GetExtensionIndex(const char(&extName)[Size]) noexcept;
		// To be used with dynamically allocated string (slower than normal version)
		static U16 GetExtensionIndexDynamic(const char* extName)  noexcept;

	public:
		Device() = default;
		Device(U32 descriptorCount, U32 scratchDescriptorCount);
		ZE_CLASS_DELETE(Device);
		~Device();

		constexpr std::pair<U32, U32> GetData() const noexcept { return { 0, 0 }; }

		constexpr U64 GetMainFence() const noexcept { return 0; }
		constexpr U64 GetComputeFence() const noexcept { return 0; }
		constexpr U64 GetCopyFence() const noexcept { return 0; }

		constexpr void WaitMain(U64 val) {}
		constexpr void WaitCompute(U64 val) {}
		constexpr void WaitCopy(U64 val) {}

		constexpr U64 SetMainFenceCPU() { return 0; }
		constexpr U64 SetComputeFenceCPU() { return 0; }
		constexpr U64 SetCopyFenceCPU() { return 0; }

		constexpr void WaitMainFromCompute(U64 val) {}
		constexpr void WaitMainFromCopy(U64 val) {}
		constexpr void WaitComputeFromMain(U64 val) {}
		constexpr void WaitComputeFromCopy(U64 val) {}
		constexpr void WaitCopyFromMain(U64 val) {}
		constexpr void WaitCopyFromCompute(U64 val) {}

		constexpr U64 SetMainFence() { return 0; }
		constexpr U64 SetComputeFence() { return 0; }
		constexpr U64 SetCopyFence() { return 0; }

#if _ZE_GFX_MARKERS
		void TagBeginMain(const wchar_t* tag, Pixel color) const noexcept {}
		void TagBeginCompute(const wchar_t* tag, Pixel color) const noexcept {}
		void TagBeginCopy(const wchar_t* tag, Pixel color) const noexcept {}

		void TagEndMain() const noexcept {}
		void TagEndCompute() const noexcept {}
		void TagEndCopy() const noexcept {}
#endif

		constexpr void BeginUploadRegion() {}
		constexpr void StartUpload() {}
		constexpr void EndUploadRegion() {}

		void ExecuteMain(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API) {}
		void ExecuteCompute(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API) {}
		void ExecuteCopy(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API) {}

		void Execute(GFX::CommandList* cls, U32 count) noexcept(!_ZE_DEBUG_GFX_API);

		// Gfx API Internal

		// Use only with VK_*_EXTENSION_NAME macros or string literals that resides underneath them
		template<U64 Size>
		constexpr bool IsExtensionSupported(const char(&extName)[Size]) const noexcept { return extensionSupport[GetExtensionIndex<Size>(extName)]; }
		// To be used with dynamically allocated string (slower than normal version)
		bool IsExtensionSupportedDynamic(const char* extName) const noexcept { return extensionSupport[GetExtensionIndexDynamic(extName)]; }
	};

#pragma region Functions
	template<U64 Size>
	constexpr U16 Device::GetExtensionIndex(const char(&extName)[Size]) noexcept
	{
		// __COUNTER__ is supported on MSVC, GCC and Clang, on other compilers should find alternative
		constexpr U16 COUNTER_BASE = __COUNTER__ + 1;

		switch (Utils::GetStringHash(extName, Size))
		{
#define X(ext) case Utils::GetStringHash(ext, sizeof(ext)): return __COUNTER__ - COUNTER_BASE;
			ZE_VK_EXT_LIST
#undef X
		default:
			return KNOWN_EXTENSION_COUNT;
		}
	}
#pragma endregion
}