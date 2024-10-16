#pragma once
#if _ZE_RHI_DX12
#	include "RHI/DX12/DiskManager.h"
#endif
// Rest of the platforms with generic disk manager implementations
#if _ZE_PLATFORM_WINDOWS
#	include "Platform/WinAPI/DiskManager.h"
namespace ZE::RHI
{
#	if _ZE_RHI_DX11
	namespace DX11
	{
		typedef ZE::WinAPI::DiskManager DiskManager;
	}
#	endif
#	if _ZE_RHI_GL
	namespace GL
	{
		typedef ZE::WinAPI::DiskManager DiskManager;
	}
#	endif
#	if _ZE_RHI_VK
	namespace VK
	{
		typedef ZE::WinAPI::DiskManager DiskManager;
	}
#	endif
}
#else
#	error Missing DiskManager platform specific implementation!
#endif

namespace ZE::IO
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

		constexpr void StartUploadGPU(bool waitable) noexcept { ZE_RHI_BACKEND_CALL(StartUploadGPU, waitable); }
		// Check if command list passed to `WaitForUploadGPU()` will need to perform any work (if needs to be open before passing to function and later executed)
		constexpr bool IsGPUWorkPending() const noexcept { bool status = false; ZE_RHI_BACKEND_CALL_RET(status, IsGPUWorkPending); return status; }
		// Before passing in command list, check `IsGPUWorkPending()`
		constexpr bool WaitForUploadGPU(GFX::Device& dev, GFX::CommandList& cl) { bool status = false; ZE_RHI_BACKEND_CALL_RET(status, WaitForUploadGPU, dev, cl); return status; }
	};
}