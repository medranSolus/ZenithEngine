#pragma once
#include "Window/MainWindow.h"
#include "D3D11.h"

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::GFX::API::DX11
{
	class Device final
	{
#if _ZE_DEBUG_GFX_API
		DX::DebugInfoManager debugManager;
		DX::ComPtr<ID3DUserDefinedAnnotation> tagManager;
#endif
		DX::ComPtr<IDevice> device;
		DX::ComPtr<IDeviceContext> context;

		U32 descriptorCount;
		U32 scratchDescriptorCount;

#if _ZE_GFX_MARKERS
		void TagBegin(std::string_view tag) const noexcept { std::wstring label = Utils::ToUtf8(tag); tagManager->BeginEvent(label.c_str()); }
#endif

		void Execute(GFX::CommandList& cl);

	public:
		Device() = default;
		Device(const Window::MainWindow& window, U32 descriptorCount, U32 scratchDescriptorCount);
		ZE_CLASS_DELETE(Device);
		~Device() = default;

		constexpr std::pair<U32, U32> GetData() const noexcept { return { descriptorCount, scratchDescriptorCount }; }

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