#include "GFX/API/DX/DirectXException.h"

namespace ZE::GFX::API::DX
{
#if _ZE_DEBUG_GFX_API
	std::string DirectXException::GetDebugInfo() const noexcept
	{
		std::ostringstream stream;
		size_t size = debugInfo.size();
		if (size > 0)
		{
			stream << "\n[Debug Error Info]";
			size_t i = 0;
			do
			{
				stream << "\n[" << i + 1 << "] " << debugInfo.at(i);
			} while (i++ < size);
			stream << std::endl;
		}
		return stream.str();
	}
#endif

	const char* DirectXException::what() const noexcept
	{
		std::ostringstream stream;
		stream << WinApiException::what();
#if _ZE_DEBUG_GFX_API
		stream << GetDebugInfo();
#endif
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}