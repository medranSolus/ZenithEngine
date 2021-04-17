#include "Exception/DirectXTexException.h"

namespace Exception
{
	const char* DirectXTexException::what() const noexcept
	{
		std::ostringstream stream;
		stream << WinApiException::what()
			<< "\n[Image Info] " << GetImageInfo();
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}