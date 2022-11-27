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

		void Execute(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API);

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

#if _ZE_GFX_MARKERS
		void TagBeginMain(const wchar_t* tag, Pixel color) const noexcept { tagManager->BeginEvent(tag); }
		void TagBeginCompute(const wchar_t* tag, Pixel color) const noexcept { tagManager->BeginEvent(tag); }
		void TagBeginCopy(const wchar_t* tag, Pixel color) const noexcept { tagManager->BeginEvent(tag); }

		void TagEndMain() const noexcept { tagManager->EndEvent(); }
		void TagEndCompute() const noexcept { tagManager->EndEvent(); }
		void TagEndCopy() const noexcept { tagManager->EndEvent(); }
#endif

		constexpr void BeginUploadRegion() {}
		constexpr void StartUpload() {}
		constexpr void EndUploadRegion() {}

		void ExecuteMain(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API) { Execute(cl); }
		void ExecuteCompute(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API) { Execute(cl); }
		void ExecuteCopy(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API) { Execute(cl); }

		void Execute(GFX::CommandList* cls, U32 count) noexcept(!_ZE_DEBUG_GFX_API);

		// Gfx API Internal

#if _ZE_DEBUG_GFX_API
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		constexpr const DX::ComPtr<IDevice>& GetDev() const noexcept { return device; }
		IDevice* GetDevice() const noexcept { return device.Get(); }
		IDeviceContext* GetMainContext() const noexcept { return context.Get(); }
	};
}