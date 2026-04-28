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
		ZE_CLASS_MOVE(DiskManager);
		~DiskManager() = default;

		static Expected<DiskManager> Create(Device& dev) noexcept { ZE_RHI_BACKEND_CREATE(DiskManager, dev); }
		ZE_RHI_BACKEND_GET(DiskManager);

		// Main Gfx API

		// After setting upload wait point it's needed to start upload of the data with 'StartUploadGPU()'
		constexpr Expected<DiskStatusHandle> SetGPUUploadWaitPoint() noexcept { ZE_RHI_BACKEND_CALL_RET(SetGPUUploadWaitPoint); }
		// Kicks off GPU work for uploading data from the CPU
		constexpr void StartUploadGPU() const noexcept { ZE_RHI_BACKEND_CALL(StartUploadGPU); }
		// Check if command list passed to `WaitForUploadGPU()` will need to perform any work (if needs to be open before passing to function and later executed)
		constexpr bool IsGPUWorkPending(DiskStatusHandle handle) const noexcept { ZE_RHI_BACKEND_CALL_RET(IsGPUWorkPending, handle); }
		// Before passing in command list, check `IsGPUWorkPending()` and make sure that you have started GPU upload with `StartUploadGPU()`
		Status WaitForUploadGPU(Device& dev, CommandList& cl, DiskStatusHandle handle) noexcept { ZE_RHI_BACKEND_CALL_RET(WaitForUploadGPU, dev, cl, handle); }
	};
}