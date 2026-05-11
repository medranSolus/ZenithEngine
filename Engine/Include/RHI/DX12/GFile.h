#pragma once
#include "GFX/DiskManager.h"

namespace ZE::RHI::DX12
{
	class GFile final
	{
		DX::ComPtr<IStorageFile> file = nullptr;

	public:
		GFile() = default;
		ZE_CLASS_MOVE(GFile);
		~GFile() = default;

		static Expected<GFile> Create() noexcept { return {}; }

		Status Open(GFX::DiskManager& disk, std::string_view fileName) noexcept;

		// Gfx API Internal

		IStorageFile* GetStorageFile() const noexcept { return file.Get(); }
	};
}