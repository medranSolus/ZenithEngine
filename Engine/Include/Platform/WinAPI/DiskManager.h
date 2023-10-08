#pragma once
#include "GFX/Device.h"

namespace ZE::WinAPI
{
	// DiskManager implementation for Windows
	class DiskManager final
	{
	public:
		DiskManager() = default;
		constexpr DiskManager(GFX::Device& dev) noexcept {}
		ZE_CLASS_MOVE(DiskManager);
		~DiskManager() = default;

		constexpr void StartUploadGPU(bool waitable) noexcept {}
		constexpr bool WaitForUploadGPU() { return true; }
	};
}