#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX
{
#ifdef _ZE_MODE_DEBUG
	std::string GraphicsException::GetDebugInfo() const noexcept
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
#endif

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