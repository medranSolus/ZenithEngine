#pragma once
#include "GFX/DiskManager.h"

namespace ZE::RHI::DX11
{
	class GFile final
	{
		IO::File file;
		Ptr<U8> memory;

	public:
		GFile() = default;
		ZE_CLASS_MOVE(GFile);
		~GFile() = default;

		static Expected<GFile> Create() noexcept { return {}; }

		Status Open(GFX::DiskManager& disk, std::string_view fileName) noexcept { return file.Open(fileName, IO::FileFlag::ReadMode | IO::FileFlag::RandomAccess, &memory); }

		// Gfx API Internal

		constexpr U8* GetMemory() const noexcept { return memory; }
	};
}