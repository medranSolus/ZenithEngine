#pragma once
#include "D3D11.h"

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::GFX::API::DX11
{
	class Device final
	{
#ifdef _ZE_MODE_DEBUG
		DX::DebugInfoManager debugManager;
#endif
		DX::ComPtr<ID3D11Device5> device;
		DX::ComPtr<ID3D11DeviceContext4> context;

		void Execute(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG);

	public:
		Device();
		Device(Device&&) = default;
		Device(const Device&) = delete;
		Device& operator=(Device&&) = default;
		Device& operator=(const Device&) = delete;
		~Device();

		constexpr U64 GetMainFence() const noexcept { return 0; }
		constexpr U64 GetComputeFence() const noexcept { return 0; }
		constexpr U64 GetCopyFence() const noexcept { return 0; }

		constexpr void WaitMain(U64 val) {}
		constexpr void WaitCompute(U64 val) {}
		constexpr void WaitCopy(U64 val) {}

		constexpr void WaitMainFromCompute(U64 val) {}
		constexpr void WaitMainFromCopy(U64 val) {}
		constexpr void WaitComputeFromMain(U64 val) {}
		constexpr void WaitComputeFromCopy(U64 val) {}
		constexpr void WaitCopyFromMain(U64 val) {}
		constexpr void WaitCopyFromCompute(U64 val) {}

		constexpr U64 SetMainFenceFromCompute() { return 0; }
		constexpr U64 SetMainFenceFromCopy() { return 0; }
		constexpr U64 SetComputeFenceFromMain() { return 0; }
		constexpr U64 SetComputeFenceFromCopy() { return 0; }
		constexpr U64 SetCopyFenceFromMain() { return 0; }
		constexpr U64 SetCopyFenceFromCompute() { return 0; }

		constexpr void FinishUpload();

		void ExecuteMain(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) { Execute(cl); }
		void ExecuteCompute(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) { Execute(cl); }
		void ExecuteCopy(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) { Execute(cl); }

		// Gfx API Internal

#ifdef _ZE_MODE_DEBUG
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		ID3D11Device5* GetDevice() const noexcept { return device.Get(); }
		ID3D11DeviceContext4* GetMainContext() const noexcept { return context.Get(); }
	};
}