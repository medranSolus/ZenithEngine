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
		ZE_CLASS_DELETE(Device);
		~Device() = default;

		constexpr void Init(const Window::MainWindow& window, U32 descriptorCount) { ZE_RHI_BACKEND_VAR.Init(window, descriptorCount); }
		constexpr void SwitchApi(GfxApiType nextApi, const Window::MainWindow& window) { U32 data; ZE_RHI_BACKEND_VAR.Switch(nextApi, window, data); }
		ZE_RHI_BACKEND_GET(Device);

		// Main Gfx API

		constexpr FfxInterface& GetFfxInterface() noexcept { FfxInterface* ffx = nullptr; ZE_RHI_BACKEND_CALL_RET(ffx, GetFfxInterface); return *ffx; }

		constexpr U64 GetMainFence() const noexcept { U64 val; ZE_RHI_BACKEND_CALL_RET(val, GetMainFence); return val; }
		constexpr U64 GetComputeFence() const noexcept { U64 val; ZE_RHI_BACKEND_CALL_RET(val, GetComputeFence); return val; }
		constexpr U64 GetCopyFence() const noexcept { U64 val; ZE_RHI_BACKEND_CALL_RET(val, GetCopyFence); return val; }

		// CPU side wait for main queue
		constexpr void WaitMain(U64 val) { ZE_RHI_BACKEND_CALL(WaitMain, val); }
		// CPU side wait for compute queue
		constexpr void WaitCompute(U64 val) { ZE_RHI_BACKEND_CALL(WaitCompute, val); }
		// CPU side wait for copy queue
		constexpr void WaitCopy(U64 val) { ZE_RHI_BACKEND_CALL(WaitCopy, val); }

		// Set fence for main queue from CPU
		constexpr U64 SetMainFenceCPU() { U64 val; ZE_RHI_BACKEND_CALL_RET(val, SetMainFenceCPU); return val; }
		// Set fence for compute queue from CPU
		constexpr U64 SetComputeFenceCPU() { U64 val; ZE_RHI_BACKEND_CALL_RET(val, SetComputeFenceCPU); return val; }
		// Set fence for copy queue from CPU
		constexpr U64 SetCopyFenceCPU() { U64 val; ZE_RHI_BACKEND_CALL_RET(val, SetCopyFenceCPU); return val; }

		// GPU side wait for main queue till compute queue will reach fence value
		constexpr void WaitMainFromCompute(U64 val) { ZE_RHI_BACKEND_CALL(WaitMainFromCompute, val); }
		// GPU side wait for main queue till copy queue will reach fence value
		constexpr void WaitMainFromCopy(U64 val) { ZE_RHI_BACKEND_CALL(WaitMainFromCopy, val); }
		// GPU side wait for compute queue till main queue will reach fence value
		constexpr void WaitComputeFromMain(U64 val) { ZE_RHI_BACKEND_CALL(WaitComputeFromMain, val); }
		// GPU side wait for compute queue till copy queue will reach fence value
		constexpr void WaitComputeFromCopy(U64 val) { ZE_RHI_BACKEND_CALL(WaitComputeFromCopy, val); }
		// GPU side wait for copy queue till main queue will reach fence value
		constexpr void WaitCopyFromMain(U64 val) { ZE_RHI_BACKEND_CALL(WaitCopyFromMain, val); }
		// GPU side wait for copy queue till compute queue will reach fence value
		constexpr void WaitCopyFromCompute(U64 val) { ZE_RHI_BACKEND_CALL(WaitCopyFromCompute, val); }

		// GPU side signal from main queue for it's fence
		constexpr U64 SetMainFence() { U64 val; ZE_RHI_BACKEND_CALL_RET(val, SetMainFence); return val; }
		// GPU side signal from compute queue for it's fence
		constexpr U64 SetComputeFence() { U64 val; ZE_RHI_BACKEND_CALL_RET(val, SetComputeFence); return val; }
		// GPU side signal from copy queue for it's fence
		constexpr U64 SetCopyFence() { U64 val; ZE_RHI_BACKEND_CALL_RET(val, SetCopyFence); return val; }

		constexpr ShaderModel GetMaxShaderModel() const noexcept { ShaderModel model; ZE_RHI_BACKEND_CALL_RET(model, GetMaxShaderModel); return model; }
		constexpr std::pair<U32, U32> GetWaveLaneCountRange() const noexcept { std::pair<U32, U32> minMax; ZE_RHI_BACKEND_CALL_RET(minMax, GetWaveLaneCountRange); return minMax; }
		constexpr bool IsShaderFloat16Supported() const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, IsShaderFloat16Supported); return val; }

		// Start sequence after which new resources can be created/updated and uploaded to GPU
		constexpr void BeginUploadRegion() { ZE_RHI_BACKEND_CALL(BeginUploadRegion); }
		// Send current resources to GPU resources to GPU
		constexpr void StartUpload() { ZE_RHI_BACKEND_CALL(StartUpload); }
		// Flushes all the upload data to GPU
		constexpr void EndUploadRegion() { ZE_RHI_BACKEND_CALL(EndUploadRegion); }

		constexpr void Execute(CommandList* cls, U32 count) { ZE_RHI_BACKEND_CALL(Execute, cls, count); }
		constexpr void ExecuteMain(CommandList& cl) { ZE_RHI_BACKEND_CALL(ExecuteMain, cl); }
		constexpr void ExecuteCompute(CommandList& cl) { ZE_RHI_BACKEND_CALL(ExecuteCompute, cl); }
		constexpr void ExecuteCopy(CommandList& cl) { ZE_RHI_BACKEND_CALL(ExecuteCopy, cl); }

		constexpr void EndFrame() noexcept { ZE_RHI_BACKEND_CALL(EndFrame); }

#if _ZE_GFX_MARKERS
		constexpr void TagBeginMain(std::string_view tag, Pixel color) const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagBeginMain, tag, color); } }
		constexpr void TagBeginCompute(std::string_view tag, Pixel color) const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagBeginCompute, tag, color); } }
		constexpr void TagBeginCopy(std::string_view tag, Pixel color) const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagBeginCopy, tag, color); } }

		constexpr void TagEndMain() const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagEndMain); } }
		constexpr void TagEndCompute() const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagEndCompute); } }
		constexpr void TagEndCopy() const noexcept { if (Settings::IsEnabledGfxTags()) { ZE_RHI_BACKEND_CALL(TagEndCopy); } }
#endif
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