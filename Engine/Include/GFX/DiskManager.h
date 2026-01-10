#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/DiskManager.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/DiskManager.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/DiskManager.h"
#endif

namespace ZE::GFX
{
	class DiskManager final
	{
		ZE_RHI_BACKEND(DiskManager);

	public:
		DiskManager() = default;
		constexpr DiskManager(GFX::Device& dev) { ZE_RHI_BACKEND_VAR.Init(dev); }
		ZE_CLASS_MOVE(DiskManager);
		~DiskManager() = default;

		constexpr void Init(GFX::Device& dev) { ZE_RHI_BACKEND_VAR.Init(dev); }
		constexpr void SwitchApi(GfxApiType nextApi, GFX::Device& dev) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev); }
		ZE_RHI_BACKEND_GET(DiskManager);

		// Main IO API

		// After setting upload wait point it's needed to start upload of the data with 'StartUploadGPU()'
		constexpr DiskStatusHandle SetGPUUploadWaitPoint() noexcept { DiskStatusHandle status = nullptr; ZE_RHI_BACKEND_CALL_RET(status, SetGPUUploadWaitPoint); return status; }
		// Kicks off GPU work for uploading data from the CPU
		constexpr void StartUploadGPU() noexcept { ZE_RHI_BACKEND_CALL(StartUploadGPU); }
		// Check if command list passed to `WaitForUploadGPU()` will need to perform any work (if needs to be open before passing to function and later executed)
		constexpr bool IsGPUWorkPending(DiskStatusHandle handle) const noexcept { bool status = false; ZE_RHI_BACKEND_CALL_RET(status, IsGPUWorkPending, handle); return status; }
		// Before passing in command list, check `IsGPUWorkPending()` and make sure that you have started GPU upload with `StartUploadGPU()`
		constexpr bool WaitForUploadGPU(GFX::Device& dev, GFX::CommandList& cl, DiskStatusHandle handle) { bool status = false; ZE_RHI_BACKEND_CALL_RET(status, WaitForUploadGPU, dev, cl, handle); return status; }
	};
}