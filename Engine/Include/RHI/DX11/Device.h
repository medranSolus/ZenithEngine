#pragma once
#include "GFX/ShaderModel.h"
#include "Window/MainWindow.h"
#include "CommandList.h"

namespace ZE::GFX
{
	class CommandList;
	namespace Resource
	{
		class Generic;
	}
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

		constexpr void EndFrame() noexcept {}

		xess_context_handle_t GetXeSSCtx() { ZE_FAIL("XeSS no supported for DirectX 11!"); return nullptr; }
		void InitializeXeSS(UInt2 targetRes, xess_quality_settings_t quality, U32 initFlags) { ZE_FAIL("XeSS not supported for DirectX 11!"); }
		void ExecuteXeSS(GFX::CommandList& cl, GFX::Resource::Generic& color, GFX::Resource::Generic& motionVectors,
			GFX::Resource::Generic* depth, GFX::Resource::Generic* exposure, GFX::Resource::Generic* responsive,
			GFX::Resource::Generic& output, float jitterX, float jitterY, UInt2 renderSize, bool reset) { ZE_FAIL("XeSS not supported for DirectX 11!"); }

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