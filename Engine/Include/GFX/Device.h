#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Device.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Device.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Device.h"
#endif
#include "RHI/Backend.h"

namespace ZE::GFX
{
	// Resource allocation
	class Device final
	{
		ZE_RHI_BACKEND(Device);

	public:
		Device() = default;
		ZE_CLASS_MOVE(Device);
		~Device() = default;

		static Expected<Device> Create(const Window::MainWindow& window, U32 descriptorCount) noexcept { ZE_RHI_BACKEND_CREATE(Device, window, descriptorCount); }
		ZE_RHI_BACKEND_GET(Device);

		// Main Gfx API

		void* GetHandle() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetHandle); }

		constexpr void OnMonitorChanged(const Window::MainWindow& window) { ZE_RHI_BACKEND_CALL(OnMonitorChanged, window); }
		constexpr const DisplayProperties& GetDisplayProperties() const noexcept { const DisplayProperties* props; ZE_RHI_BACKEND_CALL_RET_VAR(props, GetDisplayProperties); return *props; }

		constexpr U64 GetMainFence() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetMainFence); }
		constexpr U64 GetComputeFence() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetComputeFence); }
		constexpr U64 GetCopyFence() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetCopyFence); }

		// CPU side wait for main queue
		Status WaitMain(U64 val) const noexcept { ZE_RHI_BACKEND_CALL_RET(WaitMain, val); }
		// CPU side wait for compute queue
		Status WaitCompute(U64 val) const noexcept { ZE_RHI_BACKEND_CALL_RET(WaitCompute, val); }
		// CPU side wait for copy queue
		Status WaitCopy(U64 val) const noexcept { ZE_RHI_BACKEND_CALL_RET(WaitCopy, val); }

		// Set fence for main queue from CPU
		constexpr Expected<U64> SetMainFenceCPU() noexcept { ZE_RHI_BACKEND_CALL_RET(SetMainFenceCPU); }
		// Set fence for compute queue from CPU
		constexpr Expected<U64> SetComputeFenceCPU() noexcept  { ZE_RHI_BACKEND_CALL_RET(SetComputeFenceCPU); }
		// Set fence for copy queue from CPU
		constexpr Expected<U64> SetCopyFenceCPU() noexcept { ZE_RHI_BACKEND_CALL_RET(SetCopyFenceCPU); }

		// GPU side wait for main queue till compute queue will reach fence value
		Status WaitMainFromCompute(U64 val) const noexcept { ZE_RHI_BACKEND_CALL_RET(WaitMainFromCompute, val); }
		// GPU side wait for main queue till copy queue will reach fence value
		Status WaitMainFromCopy(U64 val) const noexcept { ZE_RHI_BACKEND_CALL_RET(WaitMainFromCopy, val); }
		// GPU side wait for compute queue till main queue will reach fence value
		Status WaitComputeFromMain(U64 val) const noexcept { ZE_RHI_BACKEND_CALL_RET(WaitComputeFromMain, val); }
		// GPU side wait for compute queue till copy queue will reach fence value
		Status WaitComputeFromCopy(U64 val) const noexcept { ZE_RHI_BACKEND_CALL_RET(WaitComputeFromCopy, val); }
		// GPU side wait for copy queue till main queue will reach fence value
		Status WaitCopyFromMain(U64 val) const noexcept { ZE_RHI_BACKEND_CALL_RET(WaitCopyFromMain, val); }
		// GPU side wait for copy queue till compute queue will reach fence value
		Status WaitCopyFromCompute(U64 val) const noexcept { ZE_RHI_BACKEND_CALL_RET(WaitCopyFromCompute, val); }

		// GPU side signal from main queue for it's fence
		constexpr Expected<U64> SetMainFence() noexcept { ZE_RHI_BACKEND_CALL_RET(SetMainFence); }
		// GPU side signal from compute queue for it's fence
		constexpr Expected<U64> SetComputeFence() noexcept { ZE_RHI_BACKEND_CALL_RET(SetComputeFence); }
		// GPU side signal from copy queue for it's fence
		constexpr Expected<U64> SetCopyFence() noexcept { ZE_RHI_BACKEND_CALL_RET(SetCopyFence); }

		constexpr ShaderModel GetMaxShaderModel() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetMaxShaderModel); }
		constexpr std::pair<U32, U32> GetWaveLaneCountRange() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetWaveLaneCountRange); }
		constexpr bool IsShaderFloat16Supported() const noexcept { ZE_RHI_BACKEND_CALL_RET(IsShaderFloat16Supported); }
		constexpr bool IsCoherentMemorySupported() const noexcept { ZE_RHI_BACKEND_CALL_RET(IsCoherentMemorySupported); }
		constexpr bool IsDedicatedAllocSupported() const noexcept { ZE_RHI_BACKEND_CALL_RET(IsDedicatedAllocSupported); }
		constexpr bool IsBufferMarkersSupported() const noexcept { ZE_RHI_BACKEND_CALL_RET(IsBufferMarkersSupported); }
		constexpr bool IsExtendedSynchronizationSupported() const noexcept { ZE_RHI_BACKEND_CALL_RET(IsExtendedSynchronizationSupported); }
		constexpr bool IsUavNonUniformIndexing() const noexcept { ZE_RHI_BACKEND_CALL_RET(IsUavNonUniformIndexing); }

		constexpr void Execute(CommandList* cls, U32 count) const noexcept { ZE_RHI_BACKEND_CALL(Execute, cls, count); }
		constexpr void ExecuteMain(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(ExecuteMain, cl); }
		constexpr void ExecuteCompute(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(ExecuteCompute, cl); }
		constexpr void ExecuteCopy(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(ExecuteCopy, cl); }

		constexpr Expected<FfxBreadcrumbsBlockData> AllocBreadcrumbsBlock(U64 bytes) noexcept { ZE_RHI_BACKEND_CALL_RET(AllocBreadcrumbsBlock, bytes); }
		constexpr void FreeBreadcrumbsBlock(FfxBreadcrumbsBlockData& block) noexcept { ZE_RHI_BACKEND_CALL(FreeBreadcrumbsBlock, block); }

		constexpr void EndFrame() noexcept { ZE_RHI_BACKEND_CALL(EndFrame); }

#if _ZE_GFX_MARKERS
		constexpr void TagBeginMain(std::string_view tag, Pixel color) const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagBeginMain, tag, color); } }
		constexpr void TagBeginCompute(std::string_view tag, Pixel color) const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagBeginCompute, tag, color); } }
		constexpr void TagBeginCopy(std::string_view tag, Pixel color) const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagBeginCopy, tag, color); } }

		constexpr void TagEndMain() const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagEndMain); } }
		constexpr void TagEndCompute() const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagEndCompute); } }
		constexpr void TagEndCopy() const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagEndCopy); } }
#endif

		// CPU side wait for all the commands to finish on GPU
		Status FlushGPU() noexcept;
	};
}

#if _ZE_GFX_MARKERS
#	define ZE_DRAW_TAG_BEGIN_MAIN(dev, tag, color) dev.TagBeginMain(tag, color)
#	define ZE_DRAW_TAG_BEGIN_COMPUTE(dev, tag, color) dev.TagBeginCompute(tag, color)
#	define ZE_DRAW_TAG_BEGIN_COPY(dev, tag, color) dev.TagBeginCopy(tag, color)

#	define ZE_DRAW_TAG_END_MAIN(dev) dev.TagEndMain()
#	define ZE_DRAW_TAG_END_COMPUTE(dev) dev.TagEndCompute()
#	define ZE_DRAW_TAG_END_COPY(dev) dev.TagEndCopy()
#else
#	define ZE_DRAW_TAG_BEGIN_MAIN(dev, tag, color)
#	define ZE_DRAW_TAG_BEGIN_COMPUTE(dev, tag, color)
#	define ZE_DRAW_TAG_BEGIN_COPY(dev, tag, color)

#	define ZE_DRAW_TAG_END_MAIN(dev)
#	define ZE_DRAW_TAG_END_COMPUTE(dev)
#	define ZE_DRAW_TAG_END_COPY(dev)
#endif