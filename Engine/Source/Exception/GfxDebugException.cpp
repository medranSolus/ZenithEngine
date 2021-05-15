#include "Exception/GfxDebugException.h"

namespace ZE::Exception
{
#ifdef _ZE_MODE_DEBUG
	std::string GfxDebugException::GetDebugInfo() const noexcept
	{
		std::ostringstream stream;
		size_t size = debugInfo.size();
		if (size > 0)
		{
			stream << "\n[Debug Error Info]";
			for (size_t i = 0; i < size; ++i)
				stream << "\n[" << i + 1 << "] " << debugInfo.at(i);
			stream << std::endl;
		}
		return stream.str();
	}

	const char* GfxDebugException::what() const noexcept
	{
		std::ostringstream stream;
		stream << BasicException::what() << GetDebugInfo();
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
#endif
}