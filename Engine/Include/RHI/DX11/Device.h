#pragma once
#include "GFX/ShaderModel.h"
#include "Window/MainWindow.h"
#include "CommandList.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_types.h"
#include "amd_ags.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::RHI::DX11
{
	class Device final
	{
		static_assert(Settings::MAX_RENDER_TARGETS <= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, "Incorrect number of max render targets for pass!");

#if _ZE_DEBUG_GFX_API
		DX::DebugInfoManager debugManager;
#endif
#if _ZE_GFX_MARKERS
		DX::ComPtr<ID3DUserDefinedAnnotation> tagManager;
#endif
		DX::ComPtr<IDevice> device;
		DX::ComPtr<IDeviceContext> context;

		// Hardware specific data
		union HWData
		{
			AGSContext* AMD;

			constexpr HWData() noexcept : AMD(nullptr) {}
			ZE_CLASS_NO_COPY(HWData);
			constexpr HWData(HWData&& hw) noexcept : AMD(hw.AMD) { hw.AMD = nullptr; }
			constexpr HWData& operator=(HWData&& hw) noexcept { AMD = hw.AMD; hw.AMD = nullptr; return *this; }
			~HWData() {}

		} gpuCtx = {};

		GFX::DisplayProperties displayProps = {};

#if _ZE_GFX_MARKERS
		void TagBegin(std::string_view tag) const noexcept { tagManager->BeginEvent(Utils::ToUTF16(tag).c_str()); }
#endif

		void Execute(GFX::CommandList& cl) const noexcept;
		void MoveFrom(Device&& dev) noexcept;

	public:
		Device() = default;
		ZE_CLASS_NO_COPY(Device);
		Device(Device&& dev) noexcept { MoveFrom(std::move(dev)); }
		Device& operator=(Device&& dev) noexcept { MoveFrom(std::move(dev)); return *this; }
		~Device() = default;

		static Expected<Device> Create(const Window::MainWindow& window, U32 descriptorCount) noexcept;

		constexpr GFX::ShaderModel GetMaxShaderModel() const noexcept { return GFX::ShaderModel::V5_0; }
		constexpr std::pair<U32, U32> GetWaveLaneCountRange() const noexcept { return { 32, 32 }; }
		constexpr bool IsShaderFloat16Supported() const noexcept { return false; }
		constexpr bool IsCoherentMemorySupported() const noexcept { return false; }
		constexpr bool IsDedicatedAllocSupported() const noexcept { return true; }
		constexpr bool IsBufferMarkersSupported() const noexcept { return false; }
		constexpr bool IsExtendedSynchronizationSupported() const noexcept { return false; }
		constexpr bool IsUavNonUniformIndexing() const noexcept { return true; }

		constexpr U64 GetMainFence() const noexcept { return 0; }
		constexpr U64 GetComputeFence() const noexcept { return 0; }
		constexpr U64 GetCopyFence() const noexcept { return 0; }

		constexpr void EndFrame() noexcept {}

		// There is no support for breadcrumbs in DX11 API
		constexpr Expected<FfxBreadcrumbsBlockData> AllocBreadcrumbsBlock(U64 bytes) noexcept { return FfxBreadcrumbsBlockData{}; }
		constexpr void FreeBreadcrumbsBlock(FfxBreadcrumbsBlockData& block) noexcept {}

		void* GetHandle() const noexcept { return GetDevice(); }
		const GFX::DisplayProperties* GetDisplayProperties() const noexcept { return &displayProps; }

		Status WaitMain(U64 val) const noexcept { return {}; }
		Status WaitCompute(U64 val) const noexcept { return {}; }
		Status WaitCopy(U64 val) const noexcept { return {}; }

		Expected<U64> SetMainFenceCPU() noexcept { return 0; }
		Expected<U64> SetComputeFenceCPU() noexcept { return 0; }
		Expected<U64> SetCopyFenceCPU() noexcept { return 0; }

		Status WaitMainFromCompute(U64 val) const noexcept { return {}; }
		Status WaitMainFromCopy(U64 val) const noexcept { return {}; }
		Status WaitComputeFromMain(U64 val) const noexcept { return {}; }
		Status WaitComputeFromCopy(U64 val) const noexcept { return {}; }
		Status WaitCopyFromMain(U64 val) const noexcept { return {}; }
		Status WaitCopyFromCompute(U64 val) const noexcept { return {}; }

		Expected<U64> SetMainFence() noexcept { return 0; }
		Expected<U64> SetComputeFence() noexcept { return 0; }
		Expected<U64> SetCopyFence() noexcept { return 0; }

#if _ZE_GFX_MARKERS
		void TagBeginMain(std::string_view tag, Pixel color) const noexcept { TagBegin(tag); }
		void TagBeginCompute(std::string_view tag, Pixel color) const noexcept { TagBegin(tag); }
		void TagBeginCopy(std::string_view tag, Pixel color) const noexcept { TagBegin(tag); }

		void TagEndMain() const noexcept { tagManager->EndEvent(); }
		void TagEndCompute() const noexcept { tagManager->EndEvent(); }
		void TagEndCopy() const noexcept { tagManager->EndEvent(); }
#endif

		void ExecuteMain(GFX::CommandList& cl) const noexcept { Execute(cl); }
		void ExecuteCompute(GFX::CommandList& cl) const noexcept { Execute(cl); }
		void ExecuteCopy(GFX::CommandList& cl) const noexcept { Execute(cl); }

		void OnMonitorChanged(const Window::MainWindow& window) noexcept { displayProps = DX::GetDisplayProperties(window.GetHandle()); }

		void Execute(GFX::CommandList* cls, U32 count) const noexcept;

		// Gfx API Internal

#if _ZE_DEBUG_GFX_API
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		constexpr const DX::ComPtr<IDevice>& GetDev() const noexcept { return device; }
		IDevice* GetDevice() const noexcept { return device.Get(); }
		IDeviceContext* GetMainContext() const noexcept { return context.Get(); }
	};
}