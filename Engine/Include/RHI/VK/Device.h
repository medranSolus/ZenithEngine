#pragma once
#include "GFX/ShaderModel.h"
#include "AllocatorGPU.h"
#include "CommandList.h"
#include "UploadInfo.h"

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::RHI::VK
{
	class Device final
	{
		static constexpr U16 COPY_LIST_GROW_SIZE = 5;

		struct UploadInfo
		{
			U32 LastUsedQueue;
			bool IsBuffer;
			union
			{
				VkBuffer DestBuffer;
				VkImage DestImage;
			};
			VkPipelineStageFlags2 DestStage;
			VkAccessFlags2 DestAccess;
			union
			{
				VkBuffer StagingBuffer;
				VkImage StagingImage;
			};
			Allocation StagingAlloc;
		};
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
		struct RequiredExtensionFeatures
		{
			VkPhysicalDeviceFeatures2 Features;
			VkPhysicalDeviceTimelineSemaphoreFeatures TimelineSemaphore;
			VkPhysicalDeviceSynchronization2Features Sync2;
			VkPhysicalDeviceDynamicRenderingFeatures DynamicRendering;
			VkPhysicalDeviceMultiviewFeatures Multiview;
			VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT PrimitiveRestart;
			VkPhysicalDeviceExtendedDynamicStateFeaturesEXT DynamicState;
			VkPhysicalDeviceImagelessFramebufferFeatures ImagelessFramebuffer;
			VkPhysicalDeviceDescriptorIndexingFeatures DescriptorIndexing;
			VkPhysicalDeviceDescriptorBufferFeaturesEXT DescriptorBuffer;
			VkPhysicalDeviceBufferDeviceAddressFeatures BufferAddress;
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

		U32 commandListsCount = 0;
		Ptr<VkCommandBufferSubmitInfo> commandLists;

		UA64 gfxFenceVal = 0;
		VkSemaphore gfxFence = VK_NULL_HANDLE;
		UA64 computeFenceVal = 0;
		VkSemaphore computeFence = VK_NULL_HANDLE;
		UA64 copyFenceVal = 0;
		VkSemaphore copyFence = VK_NULL_HANDLE;

		// Is integrated | Present from compute | Memory model device scope | Memory model chains | Compute full subgroups
		// Multiview GS | Multiview Tesselation | Patch list index restart | Desc buffer image layout ignored | Desc buffer capture replay
		// Buffer capture replay | Buffer address for multiple GPUs
		std::bitset<12> flags = 0;
		VkPhysicalDeviceLimits limits;
		U64 descBufferAlignment = 0;
		float conservativeRasterOverestimateSize = 0.0f;

		AllocatorGPU allocator;
		FfxInterface ffxInterface;

		CommandList copyList;
		TableInfo<U16> copyResInfo;
		U16 copyOffset = 0;
		Ptr<UploadInfo> copyResList = nullptr;

		template<U64 Size>
		static constexpr U16 GetExtensionIndex(const char(&extName)[Size]) noexcept;
		// To be used with dynamically allocated string (slower than normal version)
		static U16 GetExtensionIndexDynamic(const char* extName)  noexcept;

		void WaitCPU(VkSemaphore fence, U64 val);
		void WaitGPU(VkSemaphore fence, VkQueue queue, U64 val);
		U64 SetFenceCPU(VkSemaphore fence, UA64& fenceVal);
		U64 SetFenceGPU(VkSemaphore fence, VkQueue queue, UA64& fenceVal);
		void Execute(VkQueue queue, CommandList& cl);

#if _ZE_DEBUG_GFX_API
		static VkBool32 VKAPI_PTR DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
#endif
#if _ZE_GFX_MARKERS
		static void BeingTag(VkQueue queue, std::string_view tag, Pixel color) noexcept;
#endif

		static bool FindQueueIndices(VkPhysicalDevice device, VkSurfaceKHR testSurface, QueueFamilyInfo& familyInfo) noexcept;
		static GpuFitness CheckGpuFitness(VkPhysicalDevice device, const VkPhysicalDeviceSurfaceInfo2KHR& testSurfaceInfo,
			const std::vector<const char*>& requiredExt, RequiredExtensionFeatures& features, QueueFamilyInfo& familyInfo);

		void SetIntegratedGPU(bool present) noexcept { flags[0] = present; }
		void SetPresentFromComputeSupport(bool enabled) noexcept { flags[1] = enabled; }
		void SetMemoryModelDeviceScope(bool enabled) noexcept { flags[2] = enabled; }
		void SetMemoryModelChains(bool enabled) noexcept { flags[3] = enabled; }
		void SetFullComputeSubgroupSupport(bool enabled) noexcept { flags[4] = enabled; }
		void SetMultiviewGeometryShaderSupport(bool enabled) noexcept { flags[5] = enabled; }
		void SetMultiviewTesselationSupport(bool enabled) noexcept { flags[6] = enabled; }
		void SetPatchListIndexRestartSupport(bool enabled) noexcept { flags[7] = enabled; }
		void SetDescriptorBufferImageLayoutIngored(bool enabled) noexcept { flags[8] = enabled; }
		void SetDescriptorBufferCaptureReplaySupported(bool enabled) noexcept { flags[9] = enabled; }
		void SetBufferCaptureReplaySupported(bool enabled) noexcept { flags[10] = enabled; }
		void SetBufferAddressMultiDeviceSupported(bool enabled) noexcept { flags[11] = enabled; }

		void InitVolk();
		void CreateInstance();
		void FindPhysicalDevice(const std::vector<const char*>& requiredExt,
			const Window::MainWindow& window, RequiredExtensionFeatures& features);

	public:
		Device() = default;
		Device(const Window::MainWindow& window, U32 descriptorCount);
		ZE_CLASS_DELETE(Device);
		~Device();

		constexpr U32 GetData() const noexcept { return 0; }
		constexpr const FfxInterface* GetFfxInterface() const noexcept { return &ffxInterface; }
		constexpr GFX::ShaderModel GetMaxShaderModel() const noexcept { return GFX::ShaderModel::V6_0; }
		constexpr std::pair<U32, U32> GetWaveLaneCountRange() const noexcept { return { 32, 32 }; }
		constexpr bool IsShaderFloat16Supported() const noexcept { return false; }

		U64 GetMainFence() const noexcept { return gfxFenceVal; }
		U64 GetComputeFence() const noexcept { return computeFenceVal; }
		U64 GetCopyFence() const noexcept { return copyFenceVal; }

		void WaitMain(U64 val) { WaitCPU(gfxFence, val); }
		void WaitCompute(U64 val) { WaitCPU(computeFence, val); }
		void WaitCopy(U64 val) { WaitCPU(copyFence, val); }

		U64 SetMainFenceCPU() { return SetFenceCPU(gfxFence, gfxFenceVal); }
		U64 SetComputeFenceCPU() { return SetFenceCPU(computeFence, computeFenceVal); }
		U64 SetCopyFenceCPU() { return SetFenceCPU(copyFence, copyFenceVal); }

		void WaitMainFromCompute(U64 val) { WaitGPU(computeFence, gfxQueue, val); }
		void WaitMainFromCopy(U64 val) { WaitGPU(copyFence, gfxQueue, val); }
		void WaitComputeFromMain(U64 val) { WaitGPU(gfxFence, computeQueue, val); }
		void WaitComputeFromCopy(U64 val) { WaitGPU(copyFence, computeQueue, val); }
		void WaitCopyFromMain(U64 val) { WaitGPU(gfxFence, copyQueue, val); }
		void WaitCopyFromCompute(U64 val) { WaitGPU(computeFence, copyQueue, val); }

		U64 SetMainFence() { return SetFenceGPU(gfxFence, gfxQueue, gfxFenceVal); }
		U64 SetComputeFence() { return SetFenceGPU(computeFence, computeQueue, computeFenceVal); }
		U64 SetCopyFence() { return SetFenceGPU(computeFence, computeQueue, computeFenceVal); }

#if _ZE_GFX_MARKERS
		void TagBeginMain(std::string_view tag, Pixel color) const noexcept { BeingTag(gfxQueue, tag, color); }
		void TagBeginCompute(std::string_view tag, Pixel color) const noexcept { BeingTag(computeQueue, tag, color); }
		void TagBeginCopy(std::string_view tag, Pixel color) const noexcept { BeingTag(copyQueue, tag, color); }

		void TagEndMain() const noexcept { vkQueueEndDebugUtilsLabelEXT(gfxQueue); }
		void TagEndCompute() const noexcept { vkQueueEndDebugUtilsLabelEXT(computeQueue); }
		void TagEndCopy() const noexcept { vkQueueEndDebugUtilsLabelEXT(copyQueue); }
#endif

		void BeginUploadRegion();
		void StartUpload();
		void EndUploadRegion();

		void Execute(GFX::CommandList* cls, U32 count);
		void ExecuteMain(GFX::CommandList& cl);
		void ExecuteCompute(GFX::CommandList& cl);
		void ExecuteCopy(GFX::CommandList& cl);
		void EndFrame() noexcept;

		// Gfx API Internal

		constexpr VkInstance GetInstance() const noexcept { return instance; }
		constexpr VkPhysicalDevice GetPhysicalDevice() const noexcept { return physicalDevice; }
		constexpr VkDevice GetDevice() const noexcept { return device; }

		constexpr VkQueue GetGfxQueue() const noexcept { return gfxQueue; }
		constexpr VkQueue GetComputeQueue() const noexcept { return computeQueue; }
		constexpr VkQueue GetCopyQueue() const noexcept { return copyQueue; }
		constexpr U32 GetGfxQueueIndex() const noexcept { return gfxQueueIndex; }
		constexpr U32 GetComputeQueueIndex() const noexcept { return computeQueueIndex; }
		constexpr U32 GetCopyQueueIndex() const noexcept { return copyQueueIndex; }

		constexpr bool IsIntegratedGPU() const noexcept { return flags[0]; }
		constexpr bool CanPresentFromCompute() const noexcept { return flags[1]; }
		constexpr bool IsMemoryModelDeviceScope() const noexcept { return flags[2]; }
		constexpr bool IsMemoryModelChains() const noexcept { return flags[3]; }
		constexpr bool IsFullComputeSubgroupSupport() const noexcept { return flags[4]; }
		constexpr bool IsMultiviewGeometryShaderSupport() const noexcept { return flags[5]; }
		constexpr bool IsMultiviewTesselationSupport() const noexcept { return flags[6]; }
		constexpr bool IsPatchListIndexRestartSupport() const noexcept { return flags[7]; }
		constexpr bool IsDescriptorBufferImageLayoutIngored() const noexcept { return flags[8]; }
		constexpr bool IsDescriptorBufferCaptureReplaySupported() const noexcept { return flags[9]; }
		constexpr bool IsBufferCaptureReplaySupported() const noexcept { return flags[10]; }
		constexpr bool IsBufferAddressMultiDeviceSupported() const noexcept { return flags[11]; }

		constexpr const VkPhysicalDeviceLimits& GetLimits() const noexcept { return limits; }
		constexpr float GetConservativeRasterOverestimateSize() const noexcept { return conservativeRasterOverestimateSize; }
		constexpr AllocatorGPU& GetMemory() noexcept { return allocator; }

		// Use only with VK_*_EXTENSION_NAME macros or string literals that resides underneath them
		template<U64 Size>
		constexpr bool IsExtensionSupported(const char(&extName)[Size]) const noexcept { return extensionSupport[GetExtensionIndex<Size>(extName)]; }
		// To be used with dynamically allocated string (slower than normal version)
		bool IsExtensionSupportedDynamic(const char* extName) const noexcept { return extensionSupport[GetExtensionIndexDynamic(extName)]; }

		void UploadBindBuffer(UploadInfoBuffer& uploadInfo);
		void UploadBindTexture(UploadInfoTexture& uploadInfo);
		void UpdateBuffer(UploadInfoBufferUpdate& updateInfo);
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