#pragma once
#include "GFX/API/DX/DebugInfoManager.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class Device final
	{
#ifdef _ZE_MODE_DEBUG
		DX::DebugInfoManager debugManager;
#endif
		DX::ComPtr<ID3D11Device5> device;

	public:
		Device();
		Device(Device&&) = delete;
		Device(const Device&) = delete;
		Device& operator=(Device&&) = delete;
		Device& operator=(const Device&) = delete;
		~Device();

#ifdef _ZE_MODE_DEBUG
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		ID3D11Device5* GetDevice() const noexcept { return device.Get(); }
	};
}