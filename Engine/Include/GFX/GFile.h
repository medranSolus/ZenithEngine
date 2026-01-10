#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/GFile.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/GFile.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/GFile.h"
#endif

namespace ZE::GFX
{
	// File handle for interfacing with GPU operations
	class GFile final
	{
		ZE_RHI_BACKEND(GFile);

	public:
		GFile() noexcept { ZE_RHI_BACKEND_VAR.Init(); }
		ZE_CLASS_MOVE(GFile);
		~GFile() = default;

		constexpr void SwitchApi(GfxApiType nextApi) { ZE_RHI_BACKEND_VAR.Switch(nextApi); }
		ZE_RHI_BACKEND_GET(GFile);

		// Main Gfx API

		constexpr bool Open(DiskManager& disk, std::string_view fileName) noexcept { bool val = false; ZE_RHI_BACKEND_CALL_RET(val, Open, disk, fileName); return val; }
	};
}