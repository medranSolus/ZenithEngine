#include "GFX/API/VK/VulkanException.h"

namespace ZE::GFX::API::VK
{
	const char* VulkanException::what() const noexcept
	{
		std::ostringstream stream;
		stream << BasicException::what()
			<< "\n[Error code] " << TranslateResult(result);
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}