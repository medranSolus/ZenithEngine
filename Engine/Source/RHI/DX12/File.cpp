#include "RHI/DX12/File.h"

namespace ZE::RHI::DX12
{
	bool File::Open(IO::DiskManager& disk, std::string_view fileName, IO::FileFlags flags) noexcept
	{
		std::wstring path = Utils::ToUTF16(fileName);

		if (osFile.Open(path, flags))
		{
			// Enable DirectStorage part only when using file for interfacing with GPU
			if (flags & IO::FileFlag::GpuReading)
			{
				if (SUCCEEDED(disk.Get().dx12.GetFactory()->OpenFile(path.c_str(), IID_PPV_ARGS(&file))))
					return true;
				osFile.Close();
			}
			else
				return true;
		}
		return false;
	}
}