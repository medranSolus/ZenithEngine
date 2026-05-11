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
		GFile() = default;
		ZE_CLASS_MOVE(GFile);
		~GFile() = default;

		static Expected<GFile> Create() noexcept { ZE_RHI_BACKEND_CREATE(GFile); }
		ZE_RHI_BACKEND_GET(GFile);

		// Main Gfx API

		constexpr Status Open(DiskManager& disk, std::string_view fileName) noexcept { ZE_RHI_BACKEND_CALL_RET(Open, disk, fileName); }
	};
}