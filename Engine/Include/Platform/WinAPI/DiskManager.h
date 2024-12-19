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

		constexpr DiskStatusHandle SetGPUUploadWaitPoint() noexcept { return nullptr; }
		constexpr void StartUploadGPU() noexcept {}
		constexpr bool IsGPUWorkPending(DiskStatusHandle handle) const noexcept { return false; }
		constexpr bool WaitForUploadGPU(GFX::Device& dev, GFX::CommandList& cl, DiskStatusHandle handle) { return true; }
	};
}