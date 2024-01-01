#pragma once
#include "GFX/CommandList.h"

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
		constexpr bool IsGPUWorkPending() const noexcept { return false; }
		constexpr bool WaitForUploadGPU(GFX::Device& dev, GFX::CommandList& cl) { return true; }
	};
}