#pragma once
#include <cstdio>

#if _ZE_PLATFORM_WINDOWS
#include "Platform/WinAPI/File.h"
namespace ZE { typedef WinAPI::File PlatformFile; }
#else
#	error Missing File platform specific implementation!
#endif

namespace ZE::IO
{
	// File handle allowing for general file operations
	class File final
	{
		FILE* stdFile = nullptr;
		PlatformFile platformImpl;

	public:
		File() = default;
		ZE_CLASS_MOVE(File);
		~File() { Close(); }

		// Only available when async flag is not set
		FILE* GetHandle() const noexcept { return stdFile; }

		// Set current offset for synchronous operations
		void SetOffset(U64 offset) noexcept { platformImpl.SetOffset(stdFile, offset); }

		Task<U32> ReadAsync(void* buffer, U32 size, U64 offset) noexcept { return platformImpl.ReadAsync(buffer, size, offset); }
		Task<U32> WriteAsync(const void* buffer, U32 size, U64 offset) noexcept { return platformImpl.WriteAsync(buffer, size, offset); }

		bool Read(void* buffer, U32 size) const noexcept;
		bool Write(const void* buffer, U32 size) const noexcept;

		bool Open(std::string_view fileName, FileFlags flags = Base(FileFlag::Default), U8** fileMapping = nullptr) noexcept;
		void Close(U8* fileMapping = nullptr) noexcept;
	};
}