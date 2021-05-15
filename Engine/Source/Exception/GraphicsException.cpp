#include "Exception/GraphicsException.h"

namespace ZE::Exception
{
	const char* GraphicsException::what() const noexcept
	{
		std::ostringstream stream;
		stream << WinApiException::what();
#ifdef _ZE_MODE_DEBUG
		stream << GetDebugInfo();
#endif
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}