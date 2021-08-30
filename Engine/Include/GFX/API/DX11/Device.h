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

		constexpr void WaitMain() {}
		constexpr void WaitCompute() {}
		constexpr void WaitCopy() {}

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