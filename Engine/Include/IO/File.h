#pragma once
#if _ZE_RHI_DX12
#	include "RHI/DX12/File.h"
#endif
// Rest of the platforms with generic file implementations
#if _ZE_PLATFORM_WINDOWS
#	include "Platform/WinAPI/File.h"
namespace ZE::RHI
{
#	if _ZE_RHI_DX11
	namespace DX11
	{
		typedef ZE::WinAPI::File File;
	}
#	endif
#	if _ZE_RHI_GL
	namespace GL
	{
		typedef ZE::WinAPI::File File;
	}
#	endif
#	if _ZE_RHI_VK
	namespace VK
	{
		typedef ZE::WinAPI::File File;
	}
#	endif
}
#else
#	error Missing File platform specific implementation!
#endif

namespace ZE::IO
{
	// File handle allowing for general file operations
	class File final
	{
		ZE_RHI_BACKEND(File);

	public:
		File() = default;
		ZE_CLASS_MOVE(File);
		~File() = default;

		constexpr void Init() { ZE_RHI_BACKEND_VAR.Init(); }
		constexpr void SwitchApi(GfxApiType nextApi) { ZE_RHI_BACKEND_VAR.Switch(nextApi); }
		ZE_RHI_BACKEND_GET(File);

		// Main IO API

		bool Open(IO::DiskManager& disk, std::string_view fileName, IO::FileFlags flags) noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, Open, disk, fileName, flags); return val; }
		void Close(IO::DiskManager& disk) noexcept { ZE_RHI_BACKEND_CALL(Close, disk); }

		bool Read(U8* buffer, U32 size, U64 offset) const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, Read, buffer, size, offset); return val; }
		bool Write(U8* buffer, U32 size, U64 offset) const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, Write, buffer, size, offset); return val; }

		std::future<U32> ReadAsync(U8* buffer, U32 size, U64 offset) const noexcept { std::future<U32> val; ZE_RHI_BACKEND_CALL_RET(val, ReadAsync, buffer, size, offset); return val; }
		std::future<U32> WriteAsync(U8* buffer, U32 size, U64 offset) const noexcept { std::future<U32> val; ZE_RHI_BACKEND_CALL_RET(val, WriteAsync, buffer, size, offset); return val; }
	};
}