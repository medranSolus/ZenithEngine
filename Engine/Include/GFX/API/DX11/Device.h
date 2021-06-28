#pragma once
#include "GFX/Device.h"
#include "GFX/API/DX/DebugInfoManager.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class Device : public GFX::Device
	{
#ifdef _ZE_MODE_DEBUG
		DX::DebugInfoManager debugManager;
#endif
		DX::ComPtr<ID3D11Device5> device = nullptr;

	public:
		Device();
		virtual ~Device();

#ifdef _ZE_MODE_DEBUG
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		ID3D11Device5* GetDevice() const noexcept { return device.Get(); }
	};
}