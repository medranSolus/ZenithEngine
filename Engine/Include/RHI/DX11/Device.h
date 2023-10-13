#pragma once
#include "GFX/ShaderModel.h"
#include "Window/MainWindow.h"
#include "CommandList.h"

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::RHI::DX11
{
	class Device final
	{
#if _ZE_DEBUG_GFX_API
		DX::DebugInfoManager debugManager;
#endif
#if _ZE_GFX_MARKERS
		DX::ComPtr<ID3DUserDefinedAnnotation> tagManager;
#endif
		DX::ComPtr<IDevice> device;
		DX::ComPtr<IDeviceContext> context;
		FfxInterface ffxInterface;

		U32 descriptorCount;

#if _ZE_GFX_MARKERS
		void TagBegin(std::string_view tag) const noexcept { tagManager->BeginEvent(Utils::ToUTF16(tag).c_str()); }
#endif

		void Execute(GFX::CommandList& cl);

	public:
		Device() = default;
		Device(const Window::MainWindow& window, U32 descriptorCount);
		ZE_CLASS_DELETE(Device);
		~Device() = default;

		constexpr U32 GetData() const noexcept { return descriptorCount; }
		constexpr FfxInterface* GetFfxInterface() noexcept { return &ffxInterface; }
		constexpr bool IsShaderFloat16Supported() const noexcept { return false; }

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

		constexpr GFX::ShaderModel GetMaxShaderModel() const noexcept { return GFX::ShaderModel::V5_0; }
		constexpr std::pair<U32, U32> GetWaveLaneCountRange() const noexcept { return { 32, 32 }; }

		constexpr void BeginUploadRegion() {}
		constexpr void StartUpload() {}
		constexpr void EndUploadRegion() {}
		constexpr void EndFrame() noexcept {}

		void ExecuteMain(GFX::CommandList& cl) { Execute(cl); }
		void ExecuteCompute(GFX::CommandList& cl) { Execute(cl); }
		void ExecuteCopy(GFX::CommandList& cl) { Execute(cl); }

#if _ZE_GFX_MARKERS
		void TagBeginMain(std::string_view tag, Pixel color) const noexcept { TagBegin(tag); }
		void TagBeginCompute(std::string_view tag, Pixel color) const noexcept { TagBegin(tag); }
		void TagBeginCopy(std::string_view tag, Pixel color) const noexcept { TagBegin(tag); }

		void TagEndMain() const noexcept { tagManager->EndEvent(); }
		void TagEndCompute() const noexcept { tagManager->EndEvent(); }
		void TagEndCopy() const noexcept { tagManager->EndEvent(); }
#endif

		void Execute(GFX::CommandList* cls, U32 count);

		// Gfx API Internal

#if _ZE_DEBUG_GFX_API
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		constexpr const DX::ComPtr<IDevice>& GetDev() const noexcept { return device; }
		IDevice* GetDevice() const noexcept { return device.Get(); }
		IDeviceContext* GetMainContext() const noexcept { return context.Get(); }
	};
}