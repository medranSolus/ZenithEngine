#include "Platform/WinAPI/DirectXTexException.h"

namespace ZE::WinAPI
{
	const char* DirectXTexException::what() const noexcept
	{
		std::ostringstream stream;
		stream << WinApiException::what()
			<< "\n[Info] " << GetInfo();
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}