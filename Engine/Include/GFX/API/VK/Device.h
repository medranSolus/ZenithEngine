#pragma once
#include "VK.h"
#include "AllocatorGPU.h"

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::GFX::API::VK
{
	class Device final
	{
		struct QueueFamilyInfo
		{
			U32 Gfx = UINT32_MAX;
			U32 Compute = UINT32_MAX;
			U32 Copy = UINT32_MAX;
			bool PresentFromCompute = false;
		};
		struct GpuFitness
		{
			enum class Status : U8
			{
				Good, FeaturesInsufficient, NoPresentModes, NoPixelFormats, BackbufferFormatNotSupported,
				QueuesInsufficient, NoExtensions, NotAllRequiredExtSupported
			};
			Status Status;
			U32 ExtIndex = UINT32_MAX;
		};

#define X(ext) + 1
		static constexpr U16 KNOWN_EXTENSION_COUNT = ZE_VK_EXT_LIST;
#undef X

		LibraryHandle vulkanLibModule = nullptr;
		std::bitset<KNOWN_EXTENSION_COUNT + 1> extensionSupport;
		VkInstance instance = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;

		VkQueue gfxQueue = VK_NULL_HANDLE;
		VkQueue computeQueue = VK_NULL_HANDLE;
		VkQueue copyQueue = VK_NULL_HANDLE;
		U32 gfxQueueIndex = UINT32_MAX;
		U32 computeQueueIndex = UINT32_MAX;
		U32 copyQueueIndex = UINT32_MAX;
		// Is integrated | Present from compute | Memory model device scope | Memory model chains
		std::bitset<4> flags = 0;
		VkPhysicalDeviceLimits limits;

		AllocatorGPU allocator;

		template<U64 Size>
		static constexpr U16 GetExtensionIndex(const char(&extName)[Size]) noexcept;
		// To be used with dynamically allocated string (slower than normal version)
		static U16 GetExtensionIndexDynamic(const char* extName)  noexcept;
#if _ZE_DEBUG_GFX_API
		static VkBool32 VKAPI_PTR DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
#endif
#if _ZE_GFX_MARKERS
		static void BeingTag(VkQueue queue, const std::string_view tag, Pixel color) noexcept;
#endif

		bool FindQueueIndices(VkPhysicalDevice device, VkSurfaceKHR testSurface, QueueFamilyInfo& familyInfo) noexcept;
		GpuFitness CheckGpuFitness(VkPhysicalDevice device, const VkPhysicalDeviceSurfaceInfo2KHR& testSurfaceInfo,
			const std::vector<const char*>& requiredExt, VkPhysicalDeviceFeatures2& features, QueueFamilyInfo& familyInfo);

		void SetIntegratedGPU(bool present) noexcept { flags[0] = present; }
		void SetPresentFromComputeSupport(bool enabled) noexcept { flags[1] = enabled; }
		void SetMemoryModelDeviceScope(bool enabled) noexcept { flags[2] = enabled; }
		void SetMemoryModelChains(bool enabled) noexcept { flags[3] = enabled; }

		void InitVolk();
		void CreateInstance();
		void FindPhysicalDevice(const std::vector<const char*>& requiredExt, const Window::MainWindow& window, VkPhysicalDeviceFeatures2& features);

	public:
		Device() = default;
		Device(const Window::MainWindow& window, U32 descriptorCount, U32 scratchDescriptorCount);
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

		constexpr void BeginUploadRegion() {}
		constexpr void StartUpload() {}
		constexpr void EndUploadRegion() {}

		constexpr void ExecuteMain(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API) {}
		constexpr void ExecuteCompute(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API) {}
		constexpr void ExecuteCopy(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API) {}

#if _ZE_GFX_MARKERS
		void TagBeginMain(const std::string_view tag, Pixel color) const noexcept { BeingTag(gfxQueue, tag, color); }
		void TagBeginCompute(const std::string_view tag, Pixel color) const noexcept { BeingTag(computeQueue, tag, color); }
		void TagBeginCopy(const std::string_view tag, Pixel color) const noexcept { BeingTag(copyQueue, tag, color); }

		void TagEndMain() const noexcept { vkQueueEndDebugUtilsLabelEXT(gfxQueue); }
		void TagEndCompute() const noexcept { vkQueueEndDebugUtilsLabelEXT(computeQueue); }
		void TagEndCopy() const noexcept { vkQueueEndDebugUtilsLabelEXT(copyQueue); }
#endif

		void Execute(GFX::CommandList* cls, U32 count) noexcept(!_ZE_DEBUG_GFX_API);
		void EndFrame() noexcept;

		// Gfx API Internal

		constexpr VkInstance GetInstance() const noexcept { return instance; }
		constexpr VkPhysicalDevice GetPhysicalDevice() const noexcept { return physicalDevice; }
		constexpr VkDevice GetDevice() const noexcept { return device; }
		constexpr VkQueue GetGfxQueue() const noexcept { return gfxQueue; }
		constexpr VkQueue GetComputeQueue() const noexcept { return computeQueue; }
		constexpr VkQueue GetCopyQueue() const noexcept { return copyQueue; }

		constexpr bool IsIntegratedGPU() const noexcept { return flags[0]; }
		constexpr bool CanPresentFromCompute() const noexcept { return flags[1]; }
		constexpr bool IsMemoryModelDeviceScope() const noexcept { return flags[2]; }
		constexpr bool IsMemoryModelChains() const noexcept { return flags[3]; }

		constexpr const VkPhysicalDeviceLimits& GetLimits() const noexcept { return limits; }
		constexpr AllocatorGPU& GetMemory() noexcept { return allocator; }

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