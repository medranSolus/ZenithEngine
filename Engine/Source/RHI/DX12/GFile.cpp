#include "RHI/DX12/GFile.h"

namespace ZE::RHI::DX12
{
	bool GFile::Open(GFX::DiskManager& disk, std::string_view fileName) noexcept
	{
		return SUCCEEDED(disk.Get().dx12.GetFactory()->OpenFile(Utils::ToUTF16(fileName).c_str(), IID_PPV_ARGS(&file)));
	}
}